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

User* getUserBySession(User *head, unsigned long sid) {
    if (sid == 0) return NULL; 
    User *curr = head;
    while (curr != NULL) {
        if (curr->sessionID == sid) return curr;
        curr = curr->next;
    }
    return NULL;
}

User* registerUser(User *head, const char *username, const char *password) {
    if(strlen(password) == 0 || strlen(username) == 0) return NULL;

    // 1. Check if user already exists
    User *curr = head;
    User *last = NULL;
    while (curr != NULL) {
        if(strcmp(curr->username, username) == 0) return NULL; // Already exists
        last = curr;      // Keep track of the last node we saw
        curr = curr->next;
    }

    // 2. Create the new user
    char *hashPwd = hash_password(password);
    User *newUser = (User *)calloc(1, sizeof(User));
    strcpy(newUser->username, username);
    strcpy(newUser->passwordHash, hashPwd);

    // 3. Attach to the list
    if (head->username[0] == '\0') {
        // Special case: The head node is empty (from your empty file logic)
        strcpy(head->username, username);
        strcpy(head->passwordHash, hashPwd);
        free(newUser); // We don't need the extra node, we filled the head
        newUser = head;
    } else {
        // Attach to the end of the list
        last->next = newUser;
    }

    // 4. Save and return
    saveUser(newUser, USER_FILENAME);
    return newUser;
}
User* loginUser(User *head, const char *username, const char *password){    
    if(strlen(password) == 0 || strlen(username) == 0) return NULL;
    User *curr = head;
    
    while (curr != NULL) {
        char *hashPwd = hash_password(password);
    
        if(strcmp(curr->username, username)==0&&verify_password(password, hashPwd)==1){            
            curr->sessionID = generate_session_id(curr->username);
            return curr;
        }
        curr = curr->next;
    }
    return curr;
}
void freeUser(User *user) {
    if (user == NULL) return;

    User *current = user;
    User *next_node;

    while (current != NULL) {
        printf("Clean user: %s\n", current->username);
        next_node = current->next; // Save reference to next
        free(current);             // Delete current node
        current = next_node;       // Move to next
    }
    
    user->next = NULL; 
}
int saveUser(User *head, char *fileName){
    FILE *file_ptr;

    
    file_ptr = fopen(fileName, "a");

    
    if (file_ptr == NULL) {
        printf("Error opening file!\n");        
        return 1;
    }

    User *user = head;
    while(user!=NULL){
        fprintf(file_ptr, "%s %s\n", user->username, user->passwordHash);
        user = user->next;
    }
    

    fclose(file_ptr);
}
int loadUser(User *user, char *fileName){
    FILE *file_ptr;
    char buffer[255];
    User *current = user;
    file_ptr = fopen(fileName, "r");

    
    if (file_ptr == NULL) {
        printf("Error opening file!\n");        
        return 1;
    }
    int l=0;
    while (fgets(buffer, sizeof(buffer), file_ptr) != NULL) {
        char *token = strtok(buffer, " ");
        User *newUser = (User *)calloc(1,sizeof(User));
        int c = 0;    
        while(token!=NULL){
            
            if(c==0){
                strcpy(newUser->username, token);                    
                // newUser->next = current->next;
                
            }
            if(c==1){
                token[strcspn(token, "\n")] = 0;
                strcpy(newUser->passwordHash, token);
            }
            c++;
            token = strtok(NULL, " ");
        }
        if(l==0){
            strcpy(current->username, newUser->username);
            strcpy(current->passwordHash, newUser->passwordHash);
            free(newUser);
        }else{
            current->next = newUser;
            current = newUser;
        }
        
        
        l++;
        
        
        
    }
    
    fclose(file_ptr);
    return 0;
}