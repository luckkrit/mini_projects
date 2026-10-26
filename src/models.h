#ifndef MODELS_H 
#define MODELS_H

#include "common.h"

#define TEAM_ID_SIZE 16
#define TEAM_TITLE_SIZE 100
#define PLAYER_ID_SIZE 16
#define PLAYER_TITLE_SIZE 100
#define LEAGUE_TITLE_SIZE 100
#define MAX_PLAYER 23
#define MAX_TEAM 10
#define MAX_SQUAD_SIZE 99

#define LEAGUE_FILENAME "league.txt"
#define TEAM_FILENAME "team.txt"
#define PLAYER_FILENAME "player.txt"




typedef struct player Player;

struct player {
    char taskId[PLAYER_ID_SIZE];
    char taskTitle[PLAYER_TITLE_SIZE];
    char squadNumber[MAX_SQUAD_SIZE];
    double player_price; // In million
    int isFinish;
};

typedef struct team Team;

struct team {
    char teamId[TEAM_ID_SIZE];
    char teamTitle[TEAM_TITLE_SIZE];
    struct player players[MAX_PLAYER]; 
    int totalPlayers;
};

typedef struct league League;

struct league {
    char legueTitle[LEAGUE_TITLE_SIZE];
    struct team teams[MAX_TEAM]; 
    int totalTeams;
    int scores[MAX_TEAM];
};

int loadLeague(League *league, char *leagueFileName, char *boardFileName, char *playerFileName);
int readTeam(League *league, char *teamFileName);
int saveTeam(League *league, char *teamFileName);
int readAllPlayers(League *league, char *teamFileName);
int saveAllPlayers(League *league, char *teamFileName);
int addTeam(League *league, char *teamTitle);
int addPlayer(Team *team, char *taskTitle);
Team *hasTeam(League *league, char *teamId);
Player *hasPlayer(League *league, char *teamId, char *taskId);
#endif