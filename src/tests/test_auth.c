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

    int result2 = registerUser(head, username, password, 0);
    if(result2==0){
        printf("register user: %s %s\n", username, password);
    }
    

    int result = registerUser(head, username2, password, 0);
    if(result==0){
        printf("register user: %s %s\n", username2, password);
    }
    

    User *luser = loginUser(head, username, password);
    if(luser!=NULL){
        printf("%lu\n", luser->sessionID);    
    }
    User *luser2 = getUserBySession(head, luser->sessionID);
    if(luser2!=NULL&&luser!=NULL){
        printf("%lu %lu %d\n", luser->sessionID,luser2->sessionID,luser->sessionID == luser2->sessionID);    
        int result = logoutUser(head, luser2->username);
        printf("Logout: %d\n", result);
        // recheck
        luser2 = getUserBySession(head, luser->sessionID);
        if(luser2==NULL){
            printf("Logout success\n");
        }else{
            printf("Logout fail\n");
        }
    }
    
    // 5. Get user
    printf("\nGet user...\n");
    char userDetails[GET_USER_SIZE]; 
    getUser(head, userDetails, GET_USER_SIZE);
    printf("%s\n",userDetails);

    return 0;
}