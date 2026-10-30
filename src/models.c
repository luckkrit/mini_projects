#include "models.h"

int loadModel(League *league, char *teamFileName, char *playerFileName){
    if(readAllTeams(league, teamFileName) == 1){
        return 1;
    }
    if(readAllPlayers(league, playerFileName) == 1){
        return 1;
    }
    return 0;
}
int readAllTeams(League *league, char *teamFileName){
    FILE *file_ptr = fopen(teamFileName, "r");

    if (file_ptr == NULL) {
        // File doesn't exist, let's create it!
        printf("File not found. Creating a new one...\n");
        file_ptr = fopen(teamFileName, "w+"); // w+ creates it and allows reading
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
        char *points = strtok(NULL, ",");

        // 3. Check if tokens exist before using them
        if (teamId != NULL && teamTitle != NULL)
        {
            strcpy(league->teams[i].teamId, teamId);
            strcpy(league->teams[i].teamName, teamTitle);
            league->teams[i].points = atoi(points);
            i++;
        }
    }
    league->totalTeams = i;
    flock(fd, LOCK_UN);
    fclose(file_ptr);
    return 0;
}
int saveAllTeams(League *league,  char *teamFileName){
    FILE *file_ptr;

    file_ptr = fopen(teamFileName, "w");

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
        fprintf(file_ptr, "%s,%s,%d\n", teams[i].teamId, teams[i].teamName, teams[i].points);
    }
    flock(fd, LOCK_UN);
    fclose(file_ptr);
    return 0;
}
int readAllPlayers(League *league, char *playerFileName){
    FILE *file_ptr = fopen(playerFileName, "r");

    if (file_ptr == NULL) {
        printf("File not found. Creating a new one...\n");
        file_ptr = fopen(playerFileName, "w+");
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
        char *playerId = strtok(NULL, ",");
        char *playerName = strtok(NULL, ",");
        char *squadNumber = strtok(NULL, ",");
        char *price = strtok(NULL, ",");

        if (teamId == NULL || playerId == NULL || playerName == NULL || squadNumber == NULL || price == NULL)
            continue; // skip malformed line

        // 3. Find the team with this teamId
        Team *targetTeam = NULL;
        for (int b = 0; b < league->totalTeams; b++)
        {
            if (strcmp(league->teams[b].teamId, teamId) == 0)
            {
                targetTeam = &league->teams[b];
                break;
            }
        }

        if (targetTeam == NULL)
            continue; // player belongs to a team we don't have (shouldn't normally happen)

        // 4. Add player to that team (bounds-checked)
        if (targetTeam->totalPlayers < MAX_PLAYER)
        {
            int i = targetTeam->totalPlayers;
            strcpy(targetTeam->players[i].playerId, playerId);
            strcpy(targetTeam->players[i].playerName, playerName);
            targetTeam->players[i].squadNumber = atoi(squadNumber);
            targetTeam->players[i].price = atof(price);            
            targetTeam->totalPlayers++;
        }
    }

    flock(fd, LOCK_UN);
    fclose(file_ptr);
    return 0;
}
int saveAllPlayers(League *league, char *playerFileName){
    FILE *file_ptr = fopen(playerFileName, "w");
    if (file_ptr == NULL) {
        printf("Error opening file!\n");
        return 1;
    }
    int fd = fileno(file_ptr);
    flock(fd, LOCK_EX);

    for (int b = 0; b < league->totalTeams; b++) {
        Team *team = &league->teams[b];
        for (int i = 0; i < team->totalPlayers; i++) {
            fprintf(file_ptr, "%s,%s,%s,%d,%f\n",
                    team->teamId,
                    team->players[i].playerId,
                    team->players[i].playerName,
                    team->players[i].squadNumber,
                    team->players[i].price
                );
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
    strcpy(league->teams[league->totalTeams].teamName, teamTitle);
    league->teams[league->totalTeams].points = 0;
    league->totalTeams++;
    return 0;
}
int addPlayer(Team *team, char *playerName){
    snprintf(team->players[team->totalPlayers].playerId, sizeof(team->players[team->totalPlayers].playerId), "%d", team->totalPlayers + 1);
    strcpy(team->players[team->totalPlayers].playerName, playerName);
    team->totalPlayers++;
    return 0;
}
Team *hasTeam(League *league,char *teamId){
    if(teamId==NULL || league == NULL) return NULL;
    Team *team = NULL;
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
Player *hasPlayer(League *league, char *teamId, char *playerId){
    if(teamId == NULL || playerId == NULL || league == NULL) return NULL;
    Team *team = hasTeam(league, teamId);
    if(team==NULL) return NULL;
    Player *player = NULL;
    for (int i=0;i < team->totalPlayers; i++){
        if (strcmp(team->players[i].playerId, playerId) == 0){
            player = & team->players[i];
            break;
        }
    }
    return player;
}