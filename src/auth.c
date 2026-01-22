#include "auth.h"  
#include <crypt.h>
#include <string.h>


char* hash_password(const char *password) {
    // Using a static salt for this example
    const char *salt = "$2b$12$R9h/cIPz0gi.URQHue91zu"; 
    return crypt(password, salt);
}

bool verify_password(const char *password, const char *hash) {
    if (!password || !hash) return false;
    char *new_hash = crypt(password, hash);
    if (!new_hash) return false;
    return strcmp(new_hash, hash) == 0;
}

unsigned long generate_session_id(char *username) {
    char seed[128];
    // Mix username with current time to ensure the token is different every login
    snprintf(seed, sizeof(seed), "%s%ld", username, (long)time(NULL));
    
    unsigned long hash = 5381;
    int c;
    char *ptr = seed;
    while ((c = *ptr++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

User* getUserBySession(User *head, unsigned long sid) {
    if (sid == 0) return NULL; 
    User *curr = head;
    while (curr != NULL) {
        if (curr->sessionID == sid) return curr;
        curr = curr->next;
    }
    return NULL;
}

User* registerUser(User *head,const char *username, const char *password){
    if(strlen(password) == 0 || strlen(username) == 0) return NULL;
    char *hashPwd = hash_password(password);
    User *user = (User *)malloc(sizeof(User));
    strcpy(user->username, username);
    strcpy(user->passwordHash, hashPwd);
    User *curr = head;
    while (curr != NULL) {
        curr = curr->next;
    }
    curr = user;
    // save to file
    return user;
}

User* loginUser(User *head, const char *username, const char *password){
    if(strlen(password) == 0 || strlen(username) == 0) return NULL;
    User *curr = head;
    
    while (curr != NULL) {
        
        curr = curr->next;
    }
}