#ifndef CLIENT_H
#define CLIENT_H

#define _POSIX_C_SOURCE 200112L
#define _DEFAULT_SOURCE
#define _BSD_SOURCE

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <termios.h>
#include "protocol.h"
#include "common.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT "3030"
#define SEND_MESSAGE_SIZE 2048
#define RECEIVE_MESSAGE_SIZE 4096
#define ANSWER_SIZE 100

#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)
// ANONYMOUS
#define MENU_EXIT "0"
#define MENU_VIEW_PRODUCTS "1"
#define MENU_SEARCH_PRODUCTS "2"
#define MENU_LOGIN "3"
#define MENU_REGISTER "4"
// MEMBER
#define MENU_ADD_TO_CART "2"
#define MENU_VIEW_CART "3"
#define MENU_REMOVE_CART "4"
#define MENU_CHECKOUT_CART "5"
#define MENU_VIEW_ORDER "6"
#define MENU_MEMBER_LOGOUT "7"

// typedef struct {
//     char *rawInput;
// } MenuContext;

// typedef void (*MenuHandler)(MenuContext *ctx);
typedef void (*MenuHandler)();

typedef struct {
    char *menuName;
    MenuHandler handler;
} MenuEntry;

// ANONYMOUS 
void handleViewProducts();
void handleSearchProducts();
void handleLogin();
void handleRegister();

// MEMBER
void handleAddToCart();
void handleViewCart();
void handleRemoveCart();
void handleCheckoutCart();
void handleViewOrder();
void handleMemberLogout();

void exitApp();
void getNumericInput(char *buffer, size_t size);
void getInput(char *input, size_t size);
void getPassword(char *password, size_t size);
void showMenu(int userLevel);
void prompt(char *message, char *buffer);
void pressEnterToContinue();
#endif