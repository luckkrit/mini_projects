#ifndef AUTH_H // Fix Auth is undefined in server.c
#define AUTH_H

#define _POSIX_C_SOURCE 200809L // Must be the first line
#include <stdbool.h>
#include <time.h>
#include <sys/file.h>
#include <crypt.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define HASH_SIZE 128
#define USER_FILENAME "user.txt"
#define USERNAME_SIZE 50
#define MAX_USERS 100
#define GET_USER_SIZE 2048

typedef struct User {
    char username[USERNAME_SIZE];
    char passwordHash[HASH_SIZE];
    unsigned long sessionID; // 0 = logged out
    bool isAdmin;
    bool isUsed;
} User;

typedef struct {
    User users[MAX_USERS];
} UserSessions;

// Ensure these return char* and bool
char* hash_password(const char *password);
bool verify_password(const char *password, const char *hash);
unsigned long generate_session_id(char *username);
User* getUserBySession(UserSessions *head, unsigned long sid);
int registerUser(UserSessions *head,const char *username, const char *password, bool isAdmin);
User* loginUser(UserSessions *head, const char *username, const char *password);
int logoutUser(UserSessions *head, const char *username);
int saveUser(UserSessions *head, char *fileName);
int loadUser(UserSessions *user, char *fileName);
char* getUser(UserSessions *head,char *output, size_t outputSize);


#endif