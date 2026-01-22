#include <stdbool.h>
#include <time.h>
#define HASH_SIZE 128
#define USER_FILENAME "user.txt"

typedef struct User {
    char username[50];
    char passwordHash[HASH_SIZE];
    unsigned long sessionID; // 0 = logged out
    struct User *next;
} User;

// Ensure these return char* and bool
char* hash_password(const char *password);
bool verify_password(const char *password, const char *hash);
unsigned long generate_session_id(char *username);
User* getUserBySession(User *head, unsigned long sid);
User* registerUser(User *head,const char *username, const char *password);
User* loginUser(User *head, const char *username, const char *password);
int saveUser(User *head, char *fileName);
int loadUser(User *user, char *fileName);