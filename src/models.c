#include "models.h"

int loadLeague(League *league, char *teamFileName, char *playerFileName){
    if(readTeam(league, teamFileName) == 1){
        return 1;
    }
    if(readAllPlayers(league, playerFileName) == 1){
        return 1;
    }
    return 0;
}
int readTeam(League *league, char *fileName){
    FILE *file_ptr = fopen(fileName, "r");

    if (file_ptr == NULL) {
        // File doesn't exist, let's create it!
        printf("File not found. Creating a new one...\n");
        file_ptr = fopen(fileName, "w+"); // w+ creates it and allows reading
    }

    if (file_ptr == NULL)
        return 1;

    char buffer[255];
    int i = 0;
    int fd = fileno(file_ptr);
    flock(fd, LOCK_EX);
    while (fgets(buffer, sizeof(buffer), file_ptr) != NULL && i < MAX_TEAM)
    {
        // 1. Remove newline
        buffer[strcspn(buffer, "\n")] = 0;

        // 2. Correct Tokenizing
        char *teamId = strtok(buffer, ",");
        char *teamTitle = strtok(NULL, ",");

        // 3. Check if tokens exist before using them
        if (teamId != NULL && teamTitle != NULL)
        {
            strcpy(league->teams[i].teamId, teamId);
            strcpy(league->teams[i].teamTitle, teamTitle);
            i++;
        }
    }
    league->totalTeams = i;
    flock(fd, LOCK_UN);
    fclose(file_ptr);
    return 0;
}
int saveTeam(League *league,  char *fileName){
    FILE *file_ptr;

    file_ptr = fopen(fileName, "w");

    if (file_ptr == NULL)
    {
        printf("Error opening file!\n");
        return 1;
    }
    int fd = fileno(file_ptr);
    flock(fd, LOCK_EX);
    Team *teams = league->teams;
    for (int i = 0; i < MAX_TEAM; i++)
    {
        fprintf(file_ptr, "%s,%s\n", teams[i].teamId, teams[i].teamTitle);
    }
    flock(fd, LOCK_UN);
    fclose(file_ptr);
    return 0;
}
int readAllPlayers(League *league, char *fileName){
    FILE *file_ptr = fopen(fileName, "r");

    if (file_ptr == NULL) {
        printf("File not found. Creating a new one...\n");
        file_ptr = fopen(fileName, "w+");
    }

    if (file_ptr == NULL)
        return 1;

    char buffer[255];
    int fd = fileno(file_ptr);
    flock(fd, LOCK_EX);

    while (fgets(buffer, sizeof(buffer), file_ptr) != NULL)
    {
        // 1. Remove newline
        buffer[strcspn(buffer, "\n")] = 0;

        // 2. Tokenize
        char *teamId = strtok(buffer, ",");
        char *taskId = strtok(NULL, ",");
        char *taskTitle = strtok(NULL, ",");
        char *isFinishStr = strtok(NULL, ",");

        if (teamId == NULL || taskId == NULL || taskTitle == NULL || isFinishStr == NULL)
            continue; // skip malformed line

        // 3. Find the team with this teamId
        Team *targetBoard = NULL;
        for (int b = 0; b < league->totalTeams; b++)
        {
            if (strcmp(league->teams[b].teamId, teamId) == 0)
            {
                targetBoard = &league->teams[b];
                break;
            }
        }

        if (targetBoard == NULL)
            continue; // player belongs to a team we don't have (shouldn't normally happen)

        // 4. Add player to that team (bounds-checked)
        if (targetBoard->totalPlayers < MAX_PLAYER)
        {
            int i = targetBoard->totalPlayers;
            strcpy(targetBoard->players[i].taskId, taskId);
            strcpy(targetBoard->players[i].taskTitle, taskTitle);
            targetBoard->players[i].isFinish = atoi(isFinishStr);
            targetBoard->totalPlayers++;
        }
    }

    flock(fd, LOCK_UN);
    fclose(file_ptr);
    return 0;
}
int saveAllPlayers(League *league, char *fileName){
    FILE *file_ptr = fopen(fileName, "w");
    if (file_ptr == NULL) {
        printf("Error opening file!\n");
        return 1;
    }
    int fd = fileno(file_ptr);
    flock(fd, LOCK_EX);

    for (int b = 0; b < league->totalTeams; b++) {
        Team *team = &league->teams[b];
        for (int i = 0; i < team->totalPlayers; i++) {
            fprintf(file_ptr, "%s,%s,%s,%d\n",
                    team->teamId,
                    team->players[i].taskId,
                    team->players[i].taskTitle,
                    team->players[i].isFinish);
        }
    }

    flock(fd, LOCK_UN);
    fclose(file_ptr);
    return 0;
}
int addTeam(League *league, char *teamTitle){
    
    snprintf(league->teams[league->totalTeams].teamId, 
            sizeof(league->teams[league->totalTeams].teamId), 
            "%d", league->totalTeams+1);
    strcpy(league->teams[league->totalTeams].teamTitle, teamTitle);
    league->totalTeams++;
    return 0;
}
int addPlayer(Team *team, char *taskTitle){
    snprintf(team->players[team->totalPlayers].taskId, sizeof(team->players[team->totalPlayers].taskId), "%d", team->totalPlayers + 1);
    strcpy(team->players[team->totalPlayers].taskTitle, taskTitle);
    team->totalPlayers++;
    return 0;
}
Team *hasTeam(League *league,char *teamId){
    Team *team = NULL;
    if(teamId==NULL) return NULL;
    for (int i = 0; i < league->totalTeams; i++)
    {
        if (strcmp(league->teams[i].teamId, teamId) == 0)
        {
            team = &league->teams[i];
            break;
        }
    }
    return team;
}
Player *hasPlayer(League *league, char *teamId, char *taskId){
    Team *team = hasTeam(league, teamId);
    if(team==NULL || taskId == NULL) return NULL;
    Player *player = NULL;
    for (int i=0;i < team->totalPlayers; i++){
        if (strcmp(team->players[i].taskId, taskId) == 0){
            player = & team->players[i];
            break;
        }
    }
    return player;
}