#include <stdbool.h>

// Ensure these return char* and bool
char* hash_password(const char *password);
bool verify_password(const char *password, const char *hash);

