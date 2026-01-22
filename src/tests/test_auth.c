#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "auth.h" // Since -I./src is in CFLAGS, this works

int main() {
    printf("=== Starting Bcrypt Authentication Test ===\n");

    const char *username = "Krit";
    const char *password = "password123";


    User *user = (User *)malloc(sizeof(User));
    strcpy(user->username, username);


    char *hashed_pw = hash_password(password);

    if (hashed_pw == NULL) {
        printf("[FAIL] Hashing failed\n");
        return 1;
    }

    strcpy(user->passwordHash, hashed_pw);

    printf("[PASS] Hash generated successfully.\n");

    if (verify_password(password, hashed_pw)) {
        printf("[PASS] Verification successful\n");
    } else {
        printf("[FAIL] Verification failed\n");
    }

    unsigned session_id = generate_session_id(username);
    if(session_id>0){
        user->sessionID = session_id;
        printf("Username: %s\nSession ID: %lu\n",user->username, user->sessionID);
        
        printf("[PASS] Generate session successful\n");
    }else{
        printf("[FAIL] Generate session failed\n");
    }

    if(getUserBySession(user, user->sessionID)!=NULL){
        printf("[PASS] Authorize user\n");
    }else{
        printf("[FAIL] Unauthorize user\n");
    }

    const char *username2 = "Krit2";
    const char *password2 = password;
    User *user2 = registerUser(user, username2, password2);
    if(user2!=NULL){
        printf("[PASS] Register user\n");
        printf("Username: %s\n", user2->username);
    }else{
        printf("[FAIL] Register user\n");
    }

    return 0;
}