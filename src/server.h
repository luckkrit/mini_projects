#define _POSIX_C_SOURCE 200112L
#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <netdb.h>
#include "store.h"


#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)

typedef void (*CommandHandler)(Store*, SOCKET, char*);

typedef struct {
    char *commandName;
    CommandHandler handler;
} CommandEntry;
void handleBuy(Store *store, SOCKET client, char *saveptr);
void handleView(Store *store, SOCKET client, char *saveptr);
void handleLogin(Store *store, SOCKET client, char *saveptr);
int handleClient(Store *store,SOCKET socket_client,char *read);
int setup(char *port);