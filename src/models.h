#ifndef MODELS_H 
#define MODELS_H

#include "common.h"


#define TEAM_ID_SIZE 16
#define TEAM_NAME_SIZE 100
#define PLAYER_ID_SIZE 16
#define PLAYER_NAME_SIZE 100

#define MAX_PLAYER 100
#define MAX_TEAM 10
#define MAX_SQUAD_SIZE 99

#define TEAM_FILENAME "team.txt"
#define PLAYER_FILENAME "player.txt"

typedef struct {
    char   playerId[PLAYER_ID_SIZE];
    char   playerName[PLAYER_NAME_SIZE];
    int    squadNumber;
    double price;              // in million
} Player;

typedef struct {
    char   teamId[TEAM_ID_SIZE];
    char   teamName[TEAM_NAME_SIZE];
    Player players[MAX_PLAYER];
    int    totalPlayers;
    int    points;
} Team;

typedef struct {
    Team   teams[MAX_TEAM];
    int    totalTeams;
} League;

int loaLeague(League *league, char *teamFileName, char *playerFileName);
int readAllTeams(League *league, char *teamFileName);
int saveAllTeams(League *league, char *teamFileName);
int readAllPlayers(Team *team, char *playerFileName);
int saveAllPlayers(Team *team, char *playerFileName);
int addTeam(League *league, char *teamName);
int addPlayer(Team *team, char *playerName);
Team *hasTeam(League *league, char *teamId);
Player *hasPlayer(League *league, char *teamId, char *playerId);
#endif