#ifndef SERVER_H // Fix SOCKET is undefined in server.c
#define SERVER_H

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
#include "auth.h"
#include "protocol.h"
#include "order.h"

#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)
#define SERVER_PORT "3030"
#define MESSAGE_SIZE 2000

typedef struct {
    UserSessions *sessions;
    Store *store;
    Order order;
    SOCKET clientSocket;
    char *rawInput;
} CommandContext;

typedef void (*CommandHandler)(CommandContext *ctx);

typedef struct {
    char *commandName;
    CommandHandler handler;
} CommandEntry;



// Anonymous
void handleLogin(CommandContext *ctx);
void handleLogout(CommandContext *ctx);
void handleViewProduct(CommandContext *ctx);
void handleSearchProduct(CommandContext *ctx);
void handleRegisterMember(CommandContext *ctx);

// Admin
void handleUpdateProduct(CommandContext *ctx);
void handleViewMember(CommandContext *ctx);

// Common
int handleClient(SOCKET socket_client,char *read);
void handle_shutdown(int sig);
int setup(char *port);


#endif