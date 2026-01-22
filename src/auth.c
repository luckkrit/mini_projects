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