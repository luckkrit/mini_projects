#include <stdio.h>
#include <stdlib.h>
#include "models.h"
#include <assert.h>


int main(){
    // printf("Start testing all models\n");

    // printf("Testing League\n");
    // printf("Testing add team and player to League\n");
    // League *league = calloc(1, sizeof(League));
    // if (league == NULL) {
    //     printf("calloc failed! sizeof(League) = %zu\n", sizeof(League));
    //     return 1;
    // }
    // addTeam(league, "Team 1");
    // assert(league->totalTeams == 1); // it should be 1
    // assert(isEmptyOrSpace(league->teams[0].teamId)==0); // it should not empty


    // addPlayer(&league->teams[0], "Player 1");
    // assert(league->teams[0].totalPlayers == 1); // it should be 1
    // assert(isEmptyOrSpace(league->teams[0].players[0].taskId)==0); // it should not empty

    // printf("Testing save team to file\n");
    // saveAllTeams(league, TEAM_FILENAME);
    // assert(fileExists(TEAM_FILENAME) == 0);

    // printf("Testing save players to file\n");
    // saveAllPlayers(league, PLAYER_FILENAME);
    // assert(fileExists(PLAYER_FILENAME) == 0);

    // // 4. Clean up allocated memory
    // free(league);

    // printf("Testing load team and player to League\n");
    // league = calloc(1, sizeof(League));
    // if (league == NULL) {
    //     printf("calloc failed! sizeof(League) = %zu\n", sizeof(League));
    //     return 1;
    // }
    
    // printf("Testing load team from file\n");
    // readAllTeams(league, TEAM_FILENAME);
    // assert(league->totalTeams == 1); // it should be 1
    // assert(isEmptyOrSpace(league->teams[0].teamId)==0); // it should not empty
    // assert(strcmp(league->teams[0].teamTitle, "Team 1") == 0); // it should be Team 1

    // printf("Testing load player from file\n");
    // readAllPlayers(league, PLAYER_FILENAME);
    // assert(league->teams[0].totalPlayers == 1); // it should be 1
    // assert(isEmptyOrSpace(league->teams[0].players[0].taskId)==0); // it should not empty
    // assert(strcmp(league->teams[0].players[0].taskTitle, "Player 1") == 0); // it should be Player 1

    // free(league);

    // printf("Testing load league\n");
    // league = calloc(1, sizeof(League));
    // if (league == NULL) {
    //     printf("calloc failed! sizeof(League) = %zu\n", sizeof(League));
    //     return 1;
    // }
    
    // printf("Testing load team from file\n");
    // readAllTeams(league, TEAM_FILENAME);
    // assert(league->totalTeams == 1); // it should be 1
    // assert(isEmptyOrSpace(league->teams[0].teamId)==0); // it should not empty
    // assert(strcmp(league->teams[0].teamTitle, "Team 1") == 0); // it should be Team 1

    // printf("Testing load player from file\n");
    // readAllPlayers(league, PLAYER_FILENAME);
    // assert(league->teams[0].totalPlayers == 1); // it should be 1
    // assert(isEmptyOrSpace(league->teams[0].players[0].taskId)==0); // it should not empty
    // assert(strcmp(league->teams[0].players[0].taskTitle, "Player 1") == 0); // it should be Player 1

    // free(league);
    // printf("All model tests passed successfully!\n");
    return 0;
}
