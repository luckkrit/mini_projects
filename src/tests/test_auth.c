#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "auth.h" // Since -I./src is in CFLAGS, this works

int main() {
    printf("=== Starting Bcrypt Authentication Test ===\n");

    const char *username = "Krit";
    const char *username2 = "Krit2";
    const char *password = "password123";
    const char *adminUser = "admin";
    const char *adminPwd = "admin";

    UserSessions *head = (UserSessions *)calloc(1, sizeof(UserSessions));
    if(loadUser(head, USER_FILENAME)==1){
        registerUser(head, adminUser, adminPwd, 1);        
    }

    User *user = registerUser(head, username, password, 0);
    if(user!=NULL){
        printf("register user: %s %s\n", user->username, user->passwordHash);
    }
    

    User *user2 = registerUser(head, username2, password, 0);
    if(user2!=NULL){
        printf("register user: %s %s\n", user2->username, user2->passwordHash);
    }
    

    User *luser = loginUser(head, username, password);
    if(luser!=NULL){
        printf("%lu\n", luser->sessionID);    
    }
    User *luser2 = getUserBySession(head, luser->sessionID);
    if(luser2!=NULL&&luser!=NULL){
        printf("%lu %lu %d\n", luser->sessionID,luser2->sessionID,luser->sessionID == luser2->sessionID);    
    }
    // // printf("%lu", luser->sessionID);
    // User *luser2 = getUserBySession(head, luser->sessionID);
    // printf("%lu %d\n", luser->sessionID,luser->sessionID == luser2->sessionID);

    // User *user = (User *)calloc(1,sizeof(User));    
    // loadUser(user, USER_FILENAME);
    
    // if(strcmp(user->username, "")==0 && strcmp(user->passwordHash, "")==0){        
    //     registerUser(user, username, password);
    //     printf("register user: %s %s\n", user->username, user->passwordHash);
    // }else{
    //     printf("load user: %s %s\n", user->username, user->passwordHash);
    // }

    // registerUser(user, "Krit2", password);

    // User *luser = loginUser(user, username, password);
    // // printf("%lu", luser->sessionID);
    // User *luser2 = getUserBySession(luser, luser->sessionID);
    // printf("%lu %d\n", luser->sessionID,luser->sessionID == luser2->sessionID);

    // luser = loginUser(user, "Krit2", password);
    // // printf("%lu", luser->sessionID);
    // luser2 = getUserBySession(luser, luser->sessionID);
    // printf("%lu %d\n", luser->sessionID, luser->sessionID == luser2->sessionID);

    // freeUser(user);
    


    return 0;
}