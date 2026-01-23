#include "auth.h"  
#include <crypt.h>
#include <string.h>
#include <stdio.h>


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

User* getUserBySession(UserSessions *head, unsigned long sid) {
    if (sid == 0) return NULL; 
    for(int i=0;i<MAX_USERS;i++){
        if(head->users[i].sessionID == sid){
            return &head->users[i];
        }
    }
    return NULL;
}

User* registerUser(UserSessions *head, const char *username, const char *password, bool isAdmin) {
    if(strlen(password) == 0 || strlen(username) == 0) return NULL;

    // 1. Check if user already exists
    for(int i=0;i<MAX_USERS;i++){
        
        if(head->users[i].isUsed){
            
            if(strcmp(head->users[i].username, username)==0){
                
                printf("Duplicate user\n");
                return NULL;
            }            
        }
    }

    // 2. Create the new user
    char *hashPwd = hash_password(password);
    User *newUser = NULL;
    for(int i=0;i<MAX_USERS;i++){
        if(head->users[i].username[0] == '\0'){
            strcpy(head->users[i].username, username);
            strcpy(head->users[i].passwordHash, hashPwd);
            head->users[i].isAdmin = isAdmin;
            head->users[i].isUsed = true;
            newUser = &head->users[i];
            break;
        }
    }

    
    // 3. Save and return
    saveUser(head, USER_FILENAME);
    return newUser;
}
User* loginUser(UserSessions *head, const char *username, const char *password){    
    if(strlen(password) == 0 || strlen(username) == 0) return NULL;
    
    for(int i=0;i<MAX_USERS;i++){
        char *hashPwd = hash_password(password);    
        if(strcmp(head->users[i].username, username)==0&&verify_password(password, hashPwd)==1){            
            head->users[i].sessionID = generate_session_id(head->users[i].username);
            return &head->users[i];
        }
    }
    return NULL;
}

int saveUser(UserSessions *head, char *fileName){
    FILE *file_ptr;

    
    file_ptr = fopen(fileName, "w");

    
    if (file_ptr == NULL) {
        printf("Error opening file!\n");        
        return 1;
    }

    int fd = fileno(file_ptr);
    flock(fd, LOCK_EX);
    
    for(int i=0;i<MAX_USERS;i++){
        if(head->users[i].isUsed){
            fprintf(file_ptr, "%s %s %d\n", head->users[i].username, head->users[i].passwordHash, head->users[i].isAdmin);        
        }
    }
    
    flock(fd, LOCK_UN);
    fclose(file_ptr);
    return 0;
}
int loadUser(UserSessions *head, char *fileName){
    FILE *file_ptr;
    char buffer[255];
    
    file_ptr = fopen(fileName, "r");

    
    if (file_ptr == NULL) {
        printf("Error opening file!\n");        
        return 1;
    }
    int i=0;
    int fd = fileno(file_ptr);
    flock(fd, LOCK_EX);
    while (fgets(buffer, sizeof(buffer), file_ptr) != NULL && i < MAX_USERS)
    {
        // 1. Remove newline
        buffer[strcspn(buffer, "\n")] = 0;
        

        // 2. Collect user
        char *username = strtok(buffer, " ");
        char *hashPwd = strtok(NULL, " ");
        char *isAdmin = strtok(NULL, " ");
        
        if (username != NULL && hashPwd != NULL && isAdmin != NULL)
        {
            
            strcpy(head->users[i].username, username);
            strcpy(head->users[i].passwordHash, hashPwd);
            head->users[i].isAdmin = atoi(isAdmin);
            head->users[i].isUsed = true;
            i++;
        }
    }
    
    flock(fd, LOCK_UN);
    fclose(file_ptr);
    return 0;
}