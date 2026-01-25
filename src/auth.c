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

int registerUser(UserSessions *head, const char *username, const char *password, bool isAdmin) {
    if(strlen(password) == 0 || strlen(username) == 0) return 1;

    // 1. Check if user already exists
    for(int i=0;i<MAX_USERS;i++){
        
        if(head->users[i].isUsed){
            
            if(strcmp(head->users[i].username, username)==0){
                
                printf("Duplicate user\n");
                return 2;
            }            
        }
    }

    // 2. Create the new user
    char *hashPwd = hash_password(password);
    
    for(int i=0;i<MAX_USERS;i++){
        if(head->users[i].username[0] == '\0'){
            strcpy(head->users[i].username, username);
            strcpy(head->users[i].passwordHash, hashPwd);
            head->users[i].isAdmin = isAdmin;
            head->users[i].isUsed = true;
            
            break;
        }
    }

    
    // 3. Save and return
    saveUser(head, USER_FILENAME);
    return 0;
}
User* loginUser(UserSessions *head, const char *username, const char *password) {    
    if (strlen(password) == 0 || strlen(username) == 0) return NULL;

    for (int i = 0; i < MAX_USERS; i++) {
        // --- เพิ่มการเช็คตรงนี้ ---
        // ตรวจสอบว่าช่อง i นี้มีชื่อผู้ใช้อยู่จริง (ไม่ใช่สตริงว่าง)
        if (head->users[i].username[0] == '\0') {
            continue; // ข้ามไปดูช่องถัดไป
        }

        // เช็คชื่อผู้ใช้
        if (strcmp(head->users[i].username, username) == 0) {
            // ถ้าชื่อตรง -> เช็ครหัสผ่าน
            if (verify_password(password, head->users[i].passwordHash) == 1) {
                head->users[i].sessionID = generate_session_id(head->users[i].username);
                return &head->users[i]; // Login สำเร็จ!
            } else {
                // เจอชื่อแล้วแต่รหัสผิด หยุดหาทันทีเพื่อความปลอดภัย
                return NULL; 
            }
        }
    }
    
    // วนจนครบ MAX_USERS แล้วไม่เจอชื่อที่ตรงกันเลย
    return NULL; 
}

int logoutUser(UserSessions *head, const char *username){    
    if(strlen(username) == 0) return 1;
    
    for(int i=0;i<MAX_USERS;i++){        
        if(strcmp(head->users[i].username, username)==0){            
            head->users[i].sessionID = 0;
            return 0;
        }
    }
    return 1;
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
char* getUser(UserSessions *head,char *output, size_t outputSize){

    // 1. Initialize the buffer properly
    output[0] = '\0';
    strncat(output, "", outputSize - 1);

    char lineBuffer[1000]; // Temporary buffer for each line

    for (int i = 0; i < MAX_USERS; i++)
    {
        if (head->users[i].isUsed && !head->users[i].isAdmin)
        {
            // 2. Format the line into a temporary buffer
            snprintf(lineBuffer, sizeof(lineBuffer), "%s|",
                     head->users[i].username);

            // 3. Check if there is enough space left in 'output' to append
            if (strlen(output) + strlen(lineBuffer) < outputSize - 1)
            {
                
                strcat(output, lineBuffer);
            }
            else
            {
                break; // Buffer is full
            }
        }
    }
    return output;
}