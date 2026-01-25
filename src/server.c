#include "server.h"
#include <signal.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>    /* For O_CREAT, O_RDWR constants */
#include <sys/stat.h> /* For mode constants like 0644 */

SOCKET socket_listen = -1;
Store *store = NULL;
UserSessions *user = NULL;
Cart *cart = NULL;
Order *currentOrder = NULL;
sem_t *sem = NULL;
const char *adminUser = "admin";
const char *adminPwd = "admin";

void handleRemoveProduct(CommandContext *ctx){
  char *sessionId = strtok_r(NULL, COMMAND_SEPARATOR, &ctx->rawInput);
  char *productId = strtok_r(NULL, COMMAND_SEPARATOR, &ctx->rawInput);
  char response[MESSAGE_SIZE];
  char *endptr;
  long unsigned sessionId2 = strtoul(sessionId, &endptr, 10); // base 10
  if (*endptr != '\0' && *endptr != '\n' && *endptr != '\r')
  {
    // This catches if the string contained letters or symbols
    printf("Warning: Partial conversion. Ended at: %s\n", endptr);
  }
  printf("SessionId: %s %lu\n", sessionId, sessionId2);
  User *luser = getUserBySession(ctx->sessions, sessionId2);
  if (luser == NULL)
  {
    snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_UPDATE_PRODUCT, COMMAND_SEPARATOR, STATUS_INVALID_SESSION);
    send(ctx->clientSocket, response, strlen(response), 0);
  }
  else
  {
    if (productId)
    {
      // 1. Wait for the lock (Block if another process is buying)
      sem_wait(sem);

      // 2. CRITICAL SECTION: Only one process can be here at a time
      int result = deleteProduct(ctx->store, productId);
      saveStore(ctx->store, STORE_FILENAME);
      // 3. Release the lock
      sem_post(sem);
      if (result == 0)
      {
        snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_REMOVE_PRODUCT, COMMAND_SEPARATOR, STATUS_OK);
        send(ctx->clientSocket, response, strlen(response), 0);
      }
      else
      {
        snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_REMOVE_PRODUCT, COMMAND_SEPARATOR, STATUS_FAIL);
        send(ctx->clientSocket, response, strlen(response), 0);
      }
    }
    else
    {
      snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_REMOVE_PRODUCT, COMMAND_SEPARATOR, STATUS_INVALID_ARGUMENTS);
      send(ctx->clientSocket, response, strlen(response), 0);
    }
  }
}

void handleAddProduct(CommandContext *ctx){
  char *sessionId = strtok_r(NULL, COMMAND_SEPARATOR, &ctx->rawInput);
  char *productId = strtok_r(NULL, COMMAND_SEPARATOR, &ctx->rawInput);
  char *productTitle = strtok_r(NULL, COMMAND_SEPARATOR, &ctx->rawInput);
  char *priceStr = strtok_r(NULL, COMMAND_SEPARATOR, &ctx->rawInput);
  char *qtyStr = strtok_r(NULL, COMMAND_SEPARATOR, &ctx->rawInput);
  char response[MESSAGE_SIZE];
  char *endptr;
  long unsigned sessionId2 = strtoul(sessionId, &endptr, 10); // base 10
  if (*endptr != '\0' && *endptr != '\n' && *endptr != '\r')
  {
    // This catches if the string contained letters or symbols
    printf("Warning: Partial conversion. Ended at: %s\n", endptr);
  }
  printf("SessionId: %s %lu\n", sessionId, sessionId2);
  User *luser = getUserBySession(ctx->sessions, sessionId2);
  if (luser == NULL)
  {
    snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_UPDATE_PRODUCT, COMMAND_SEPARATOR, STATUS_INVALID_SESSION);
    send(ctx->clientSocket, response, strlen(response), 0);
  }
  else
  {
    if (productId && productTitle && qtyStr && priceStr)
    {
      // 1. Wait for the lock (Block if another process is buying)
      sem_wait(sem);

      // 2. CRITICAL SECTION: Only one process can be here at a time
      int result = updateProduct(ctx->store, productId, productTitle, atof(priceStr), atoi(qtyStr));
      saveStore(ctx->store, STORE_FILENAME);
      // 3. Release the lock
      sem_post(sem);
      if (result == 0)
      {
        snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_ADD_PRODUCT, COMMAND_SEPARATOR, STATUS_OK);
        send(ctx->clientSocket, response, strlen(response), 0);
      }
      else
      {
        snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_ADD_PRODUCT, COMMAND_SEPARATOR, STATUS_FAIL);
        send(ctx->clientSocket, response, strlen(response), 0);
      }
    }
    else
    {
      snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_ADD_PRODUCT, COMMAND_SEPARATOR, STATUS_INVALID_ARGUMENTS);
      send(ctx->clientSocket, response, strlen(response), 0);
    }
  }
}
void handleViewOrder(CommandContext *ctx)
{
  char *sessionId = strtok_r(NULL, COMMAND_SEPARATOR, &ctx->rawInput);
  char response[MESSAGE_SIZE + GET_CART_ORDER_SIZE];
  char *endptr;
  long unsigned sessionId2 = strtoul(sessionId, &endptr, 10); // base 10
  if (*endptr != '\0' && *endptr != '\n' && *endptr != '\r')
  {
    // This catches if the string contained letters or symbols
    printf("Warning: Partial conversion. Ended at: %s\n", endptr);
  }
  printf("SessionId: %s %lu\n", sessionId, sessionId2);
  User *luser = getUserBySession(ctx->sessions, sessionId2);
  if (luser == NULL)
  {
    snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_VIEW_ORDER, COMMAND_SEPARATOR, STATUS_INVALID_SESSION);
    send(ctx->clientSocket, response, strlen(response), 0);
  }
  else
  {
    sem_wait(sem);

    // 1. CRITICAL SECTION: Only one process can be here at a time
    char orderDetails[GET_USER_SIZE];
    if(luser->isAdmin){
      getOrder(ctx->order,ALL_USERS, orderDetails, GET_CART_ORDER_SIZE, 1);
    }else{
      getOrder(ctx->order,luser->username, orderDetails, GET_CART_ORDER_SIZE, 1);
    }
    

    // 2. Release the lock
    sem_post(sem);
    snprintf(response, sizeof(response), "%s%s%d%s%s\n", COMMAND_VIEW_ORDER, COMMAND_SEPARATOR, STATUS_OK, COMMAND_SEPARATOR, orderDetails);
    send(ctx->clientSocket, response, strlen(response), 0);
  }
}

void handleViewCart(CommandContext *ctx)
{

  char *sessionId = strtok_r(NULL, COMMAND_SEPARATOR, &ctx->rawInput);
  char response[MESSAGE_SIZE + GET_CART_ORDER_SIZE];
  char *endptr;
  long unsigned sessionId2 = strtoul(sessionId, &endptr, 10); // base 10
  if (*endptr != '\0' && *endptr != '\n' && *endptr != '\r')
  {
    // This catches if the string contained letters or symbols
    printf("Warning: Partial conversion. Ended at: %s\n", endptr);
  }
  printf("SessionId: %s %lu\n", sessionId, sessionId2);
  User *luser = getUserBySession(ctx->sessions, sessionId2);
  if (luser == NULL)
  {
    snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_VIEW_CART, COMMAND_SEPARATOR, STATUS_INVALID_SESSION);
    send(ctx->clientSocket, response, strlen(response), 0);
  }
  else
  {
    sem_wait(sem);

    // 1. CRITICAL SECTION: Only one process can be here at a time
    char cartDetails[GET_USER_SIZE];
    if(luser->isAdmin){
      getOrder(ctx->order, ALL_USERS, cartDetails, GET_CART_ORDER_SIZE, 0);
    }else{
      getOrder(ctx->order, luser->username, cartDetails, GET_CART_ORDER_SIZE, 0);
    }
    

    // 2. Release the lock
    sem_post(sem);
    snprintf(response, sizeof(response), "%s%s%d%s%s\n", COMMAND_VIEW_CART, COMMAND_SEPARATOR, STATUS_OK, COMMAND_SEPARATOR, cartDetails);
    send(ctx->clientSocket, response, strlen(response), 0);
  }
}

void handleCheckoutCart(CommandContext *ctx)
{
  char *sessionId = strtok_r(NULL, COMMAND_SEPARATOR, &ctx->rawInput);
  char *productId = strtok_r(NULL, COMMAND_SEPARATOR, &ctx->rawInput);
  char response[MESSAGE_SIZE];
  char *endptr;
  long unsigned sessionId2 = strtoul(sessionId, &endptr, 10); // base 10
  if (*endptr != '\0' && *endptr != '\n' && *endptr != '\r')
  {
    // This catches if the string contained letters or symbols
    printf("Warning: Partial conversion. Ended at: %s\n", endptr);
  }
  printf("SessionId: %s %lu\n", sessionId, sessionId2);
  User *luser = getUserBySession(ctx->sessions, sessionId2);
  if (luser == NULL)
  {
    snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_CHECKOUT_CART, COMMAND_SEPARATOR, STATUS_INVALID_SESSION);
    send(ctx->clientSocket, response, strlen(response), 0);
  }
  else
  {
    if (productId)
    {

      // 1. Wait for the lock (Block if another process is buying)
      sem_wait(sem);

      printf("Product ID: %s\n", productId);
      // 2. CRITICAL SECTION: Only one process can be here at a time
      int result = checkoutCart(ctx->order, luser->username, productId);
      int result2 = saveOrder(ctx->order, ORDER_FILENAME);

      // 3. Release the lock
      sem_post(sem);
      if (result == 0 && result2 == 0)
      {
        snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_CHECKOUT_CART, COMMAND_SEPARATOR, STATUS_OK);
        send(ctx->clientSocket, response, strlen(response), 0);
      }
      else
      {
        snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_CHECKOUT_CART, COMMAND_SEPARATOR, STATUS_FAIL);
        send(ctx->clientSocket, response, strlen(response), 0);
      }
    }
    else
    {
      snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_CHECKOUT_CART, COMMAND_SEPARATOR, STATUS_INVALID_ARGUMENTS);
      send(ctx->clientSocket, response, strlen(response), 0);
    }
  }
}

void handleClearCart(CommandContext *ctx)
{
  char *sessionId = strtok_r(NULL, COMMAND_SEPARATOR, &ctx->rawInput);
  char response[MESSAGE_SIZE];
  char *endptr;
  long unsigned sessionId2 = strtoul(sessionId, &endptr, 10); // base 10
  if (*endptr != '\0' && *endptr != '\n' && *endptr != '\r')
  {
    // This catches if the string contained letters or symbols
    printf("Warning: Partial conversion. Ended at: %s\n", endptr);
  }
  printf("SessionId: %s %lu\n", sessionId, sessionId2);
  User *luser = getUserBySession(ctx->sessions, sessionId2);
  if (luser == NULL)
  {
    snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_UPDATE_CART, COMMAND_SEPARATOR, STATUS_INVALID_SESSION);
    send(ctx->clientSocket, response, strlen(response), 0);
  }
  else
  {
    // 1. Wait for the lock (Block if another process is buying)
    sem_wait(sem);

    // 2. CRITICAL SECTION: Only one process can be here at a time
    printf("username: %s\n", luser->username);
    int result = clearCart(ctx->order, ctx->store, luser->username);
    int result2 = saveOrder(ctx->order, ORDER_FILENAME);
    int result3 = saveStore(ctx->store, STORE_FILENAME);

    // 3. Release the lock
    sem_post(sem);
    if (result == 0 && result2 == 0 && result3 == 0)
    {
      snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_UPDATE_CART, COMMAND_SEPARATOR, STATUS_OK);
      send(ctx->clientSocket, response, strlen(response), 0);
    }
    else
    {
      snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_UPDATE_CART, COMMAND_SEPARATOR, STATUS_FAIL);
      send(ctx->clientSocket, response, strlen(response), 0);
    }
  }
}

void handleUpdateCart(CommandContext *ctx)
{
  char *sessionId = strtok_r(NULL, COMMAND_SEPARATOR, &ctx->rawInput);
  char *productId = strtok_r(NULL, COMMAND_SEPARATOR, &ctx->rawInput);
  char *qtyStr = strtok_r(NULL, COMMAND_SEPARATOR, &ctx->rawInput);
  char response[MESSAGE_SIZE];
  char *endptr;
  long unsigned sessionId2 = strtoul(sessionId, &endptr, 10); // base 10
  if (*endptr != '\0' && *endptr != '\n' && *endptr != '\r')
  {
    // This catches if the string contained letters or symbols
    printf("Warning: Partial conversion. Ended at: %s\n", endptr);
  }
  printf("SessionId: %s %lu\n", sessionId, sessionId2);
  User *luser = getUserBySession(ctx->sessions, sessionId2);
  if (luser == NULL)
  {
    snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_UPDATE_CART, COMMAND_SEPARATOR, STATUS_INVALID_SESSION);
    send(ctx->clientSocket, response, strlen(response), 0);
  }
  else
  {
    if (productId && qtyStr && atoi(qtyStr) > 0)
    {
      // 1. Wait for the lock (Block if another process is buying)
      sem_wait(sem);

      // 2. CRITICAL SECTION: Only one process can be here at a time
      int result = updateStore(ctx->store, productId, IGNORE_UPDATE_TITLE, IGNORE_UPDATE_PRICE, -1 * atoi(qtyStr));
      int result2 = updateCart(ctx->order, luser->username, productId, atoi(qtyStr));
      int result3 = saveOrder(ctx->order, ORDER_FILENAME);
      int result4 = saveStore(ctx->store, STORE_FILENAME);
      // 3. Release the lock
      sem_post(sem);
      if (result2 == 0 && result == 0 && result3 == 0 && result4 == 0)
      {
        snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_UPDATE_CART, COMMAND_SEPARATOR, STATUS_OK);
        send(ctx->clientSocket, response, strlen(response), 0);
      }
      else
      {
        snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_UPDATE_CART, COMMAND_SEPARATOR, STATUS_FAIL);
        send(ctx->clientSocket, response, strlen(response), 0);
      }
    }
    else if (productId && qtyStr && atoi(qtyStr) < 0)
    {
      // 1. Wait for the lock (Block if another process is buying)
      sem_wait(sem);

      // 2. CRITICAL SECTION: Only one process can be here at a time
      int result = updateStore(ctx->store, productId, IGNORE_UPDATE_TITLE, IGNORE_UPDATE_PRICE,-1 *  atoi(qtyStr));
      int result2 = updateCart(ctx->order, luser->username, productId, atoi(qtyStr));
      int result3 = saveOrder(ctx->order, ORDER_FILENAME);
      int result4 = saveStore(ctx->store, STORE_FILENAME);
      // 3. Release the lock
      sem_post(sem);
      if (result2 == 0 && result == 0 && result3 == 0 && result4 == 0)
      {
        snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_UPDATE_CART, COMMAND_SEPARATOR, STATUS_OK);
        send(ctx->clientSocket, response, strlen(response), 0);
      }
      else
      {
        snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_UPDATE_CART, COMMAND_SEPARATOR, STATUS_FAIL);
        send(ctx->clientSocket, response, strlen(response), 0);
      }
    }
    else
    {
      snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_UPDATE_CART, COMMAND_SEPARATOR, STATUS_INVALID_ARGUMENTS);
      send(ctx->clientSocket, response, strlen(response), 0);
    }
  }
}

void handleRegisterMember(CommandContext *ctx)
{

  char *username = strtok_r(NULL, COMMAND_SEPARATOR, &ctx->rawInput);
  char *password = strtok_r(NULL, COMMAND_SEPARATOR, &ctx->rawInput);

  char response[MESSAGE_SIZE];

  if (username && password)
  {
    // 1. Wait for the lock (Block if another process is buying)
    sem_wait(sem);

    // 2. CRITICAL SECTION: Only one process can be here at a time
    int result = registerUser(ctx->sessions, username, password, 0);
    // 3. Release the lock
    sem_post(sem);
    if (result == 0)
    {
      snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_REGISTER_MEMBER, COMMAND_SEPARATOR, STATUS_OK);
      send(ctx->clientSocket, response, strlen(response), 0);
    }
    else if (result == 2)
    {
      snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_REGISTER_MEMBER, COMMAND_SEPARATOR, STATUS_DUPLICATE_USER);
      send(ctx->clientSocket, response, strlen(response), 0);
    }
    else
    {
      snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_REGISTER_MEMBER, COMMAND_SEPARATOR, STATUS_FAIL);
      send(ctx->clientSocket, response, strlen(response), 0);
    }
  }
  else
  {
    snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_REGISTER_MEMBER, COMMAND_SEPARATOR, STATUS_INVALID_ARGUMENTS);
    send(ctx->clientSocket, response, strlen(response), 0);
  }
}

void handleViewMember(CommandContext *ctx)
{
  char response[MESSAGE_SIZE + GET_USER_SIZE];
  sem_wait(sem);

  // 1. CRITICAL SECTION: Only one process can be here at a time
  char userDetails[GET_USER_SIZE];
  getUser(ctx->sessions, userDetails, GET_USER_SIZE);

  // 2. Release the lock
  sem_post(sem);
  snprintf(response, sizeof(response), "%s%s%d%s%s\n", COMMAND_VIEW_MEMBER, COMMAND_SEPARATOR, STATUS_OK, COMMAND_SEPARATOR, userDetails);
  send(ctx->clientSocket, response, strlen(response), 0);
}

void handleSearchProduct(CommandContext *ctx)
{
  char response[MESSAGE_SIZE + GET_STORE_SIZE];
  char *productId = strtok_r(NULL, COMMAND_SEPARATOR, &ctx->rawInput);
  if (productId == NULL)
  {
    snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_SEARCH_PRODUCT, COMMAND_SEPARATOR, STATUS_INVALID_ARGUMENTS);
    send(ctx->clientSocket, response, strlen(response), 0);
  }
  else
  {

    sem_wait(sem);

    // 1. CRITICAL SECTION: Only one process can be here at a time
    char storeDetails[GET_STORE_SIZE];
    searchStore(ctx->store, productId, storeDetails, GET_STORE_SIZE);

    // 2. Release the lock
    sem_post(sem);
    snprintf(response, sizeof(response), "%s%s%d%s%s\n", COMMAND_SEARCH_PRODUCT, COMMAND_SEPARATOR, STATUS_OK, COMMAND_SEPARATOR, storeDetails);
    send(ctx->clientSocket, response, strlen(response), 0);
  }
}

void handleViewProduct(CommandContext *ctx)
{
  char response[MESSAGE_SIZE + GET_STORE_SIZE];
  sem_wait(sem);

  // 1. CRITICAL SECTION: Only one process can be here at a time
  char storeDetails[GET_STORE_SIZE];
  getStore(ctx->store, storeDetails, GET_STORE_SIZE);

  // 2. Release the lock
  sem_post(sem);
  snprintf(response, sizeof(response), "%s%s%d%s%s\n", COMMAND_VIEW_PRODUCT, COMMAND_SEPARATOR, STATUS_OK, COMMAND_SEPARATOR, storeDetails);
  send(ctx->clientSocket, response, strlen(response), 0);
}

void handleUpdateProduct(CommandContext *ctx)
{
  char *sessionId = strtok_r(NULL, COMMAND_SEPARATOR, &ctx->rawInput);
  char *productId = strtok_r(NULL, COMMAND_SEPARATOR, &ctx->rawInput);
  char *productTitle = strtok_r(NULL, COMMAND_SEPARATOR, &ctx->rawInput);
  char *priceStr = strtok_r(NULL, COMMAND_SEPARATOR, &ctx->rawInput);
  char *qtyStr = strtok_r(NULL, COMMAND_SEPARATOR, &ctx->rawInput);
  char response[MESSAGE_SIZE];
  char *endptr;
  long unsigned sessionId2 = strtoul(sessionId, &endptr, 10); // base 10
  if (*endptr != '\0' && *endptr != '\n' && *endptr != '\r')
  {
    // This catches if the string contained letters or symbols
    printf("Warning: Partial conversion. Ended at: %s\n", endptr);
  }
  printf("SessionId: %s %lu\n", sessionId, sessionId2);
  User *luser = getUserBySession(ctx->sessions, sessionId2);
  if (luser == NULL)
  {
    snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_UPDATE_PRODUCT, COMMAND_SEPARATOR, STATUS_INVALID_SESSION);
    send(ctx->clientSocket, response, strlen(response), 0);
  }
  else
  {
    if (productId && productTitle && qtyStr && priceStr)
    {
      // 1. Wait for the lock (Block if another process is buying)
      sem_wait(sem);

      // 2. CRITICAL SECTION: Only one process can be here at a time
      int result = updateProduct(ctx->store, productId, productTitle, atof(priceStr), atoi(qtyStr));
      saveStore(ctx->store, STORE_FILENAME);
      // 3. Release the lock
      sem_post(sem);
      if (result == 0)
      {
        snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_UPDATE_PRODUCT, COMMAND_SEPARATOR, STATUS_OK);
        send(ctx->clientSocket, response, strlen(response), 0);
      }
      else
      {
        snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_UPDATE_PRODUCT, COMMAND_SEPARATOR, STATUS_FAIL);
        send(ctx->clientSocket, response, strlen(response), 0);
      }
    }
    else
    {
      snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_UPDATE_PRODUCT, COMMAND_SEPARATOR, STATUS_INVALID_ARGUMENTS);
      send(ctx->clientSocket, response, strlen(response), 0);
    }
  }
}

void handleLogin(CommandContext *ctx)
{
  // printf("line: %s\n", saveptr);
  char *username = strtok_r(NULL, COMMAND_SEPARATOR, &ctx->rawInput);
  char *password = strtok_r(NULL, COMMAND_SEPARATOR, &ctx->rawInput);
  char response[MESSAGE_SIZE];

  if (username && password)
  {

    User *luser = loginUser(ctx->sessions, username, password);
    
    if (luser != NULL)
    {
      snprintf(response, sizeof(response), "%s%s%d%s%lu%s%d\n", COMMAND_LOGIN, COMMAND_SEPARATOR, STATUS_OK, COMMAND_SEPARATOR, luser->sessionID, COMMAND_SEPARATOR, luser->isAdmin);
      send(ctx->clientSocket, response, strlen(response), 0);
    }
    else
    {
      snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_LOGIN, COMMAND_SEPARATOR, STATUS_FAIL);
      send(ctx->clientSocket, response, strlen(response), 0);
    }
  }
  else
  {
    snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_LOGIN, COMMAND_SEPARATOR, STATUS_INVALID_ARGUMENTS);
    send(ctx->clientSocket, response, strlen(response), 0);
  }
}

void handleLogout(CommandContext *ctx)
{
  // printf("line: %s\n", saveptr);
  char *sessionId = strtok_r(NULL, COMMAND_SEPARATOR, &ctx->rawInput);
  char response[MESSAGE_SIZE];
  char *endptr;
  long unsigned sessionId2 = strtoul(sessionId, &endptr, 10); // base 10
  if (*endptr != '\0' && *endptr != '\n' && *endptr != '\r')
  {
    // This catches if the string contained letters or symbols
    printf("Warning: Partial conversion. Ended at: %s\n", endptr);
  }
  printf("SessionId: %s %lu\n", sessionId, sessionId2);
  User *luser = getUserBySession(ctx->sessions, sessionId2);
  if (luser == NULL)
  {
    snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_LOGOUT, COMMAND_SEPARATOR, STATUS_INVALID_SESSION);
    send(ctx->clientSocket, response, strlen(response), 0);
  }
  else
  {
    int result = logoutUser(ctx->sessions, luser->username);
    if (result == 0)
    {
      snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_LOGOUT, COMMAND_SEPARATOR, STATUS_OK);
      send(ctx->clientSocket, response, strlen(response), 0);
    }
    else
    {
      snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_LOGOUT, COMMAND_SEPARATOR, STATUS_FAIL);
      send(ctx->clientSocket, response, strlen(response), 0);
    }
  }
}

int handleClient(SOCKET socket_client, char *read)
{
  // printf("line: %s\n", read);
  char response[MESSAGE_SIZE];
  char *saveptr;
  char *commandName = strtok_r(read, ",\n\r", &saveptr);

  if (commandName == NULL)
    return -1;

  CommandContext ctx = {
      .store = store,
      .clientSocket = socket_client,
      .sessions = user,
      .order = currentOrder,
      .rawInput = saveptr,
  };

  CommandEntry commandTable[] = {
      {COMMAND_LOGIN, handleLogin},
      {COMMAND_LOGOUT, handleLogout},
      {COMMAND_UPDATE_PRODUCT, handleUpdateProduct},
      {COMMAND_VIEW_PRODUCT, handleViewProduct},
      {COMMAND_SEARCH_PRODUCT, handleSearchProduct},
      {COMMAND_VIEW_MEMBER, handleViewMember},
      {COMMAND_REGISTER_MEMBER, handleRegisterMember},
      {COMMAND_UPDATE_CART, handleUpdateCart},
      {COMMAND_CLEAR_CART, handleClearCart},
      {COMMAND_CHECKOUT_CART, handleCheckoutCart},
      {COMMAND_VIEW_CART, handleViewCart},
      {COMMAND_VIEW_ORDER, handleViewOrder},
      {NULL, NULL} // Sentinel to mark the end
  };

  for (int i = 0; commandTable[i].commandName != NULL; i++)
  {
    if (strcmp(commandName, commandTable[i].commandName) == 0)
    {
      // Found the command! Execute its function.
      printf("Found command: %s\n", commandName);
      commandTable[i].handler(&ctx);
      return 0;
    }
  }
  snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_UNKNOWN, COMMAND_SEPARATOR, STATUS_UNKNOWN);
  send(socket_client, response, strlen(response), 0);
  return -1;
}

void handle_shutdown(int sig)
{
  if (sig == SIGINT)
  {
    printf("\nCaught Ctrl+C! Cleaning up...\n");
  }
  else if (sig == SIGTERM)
  {
    printf("\nTermination request received...\n");
  }
  // To prevent an infinite loop of signals
  signal(SIGINT, SIG_IGN);

  printf("Closing listening socket...\n");
  CLOSESOCKET(socket_listen);
  socket_listen = -1;
  munmap(store, sizeof(Store));
  munmap(user, sizeof(User));
  printf("Cleaning up semaphore...\n");
  sem_close(sem);
  sem_unlink("/store_lock");

  // 2. Kill all child processes in the group
  // Passing 0 as the PID sends the signal to everyone in the process group
  kill(0, SIGTERM);

  exit(0);
}

int setup(char *port)
{
  printf("Configuring local address...\n");
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  // init semaphore
  sem = sem_open("/store_lock", O_CREAT, 0644, 1);

  // Init store,user,order using shared memory (mmap)
  store = mmap(NULL, sizeof(Store), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  loadStore(store, STORE_FILENAME);
  user = mmap(NULL, sizeof(UserSessions), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if (loadUser(user, USER_FILENAME) == 1)
  {
    registerUser(user, adminUser, adminPwd, 1);
  }
  currentOrder = mmap(NULL, sizeof(Order), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  loadOrder(currentOrder, ORDER_FILENAME);

  struct addrinfo *bind_address;
  getaddrinfo(0, port, &hints, &bind_address);

  printf("Creating socket...\n");

  socket_listen = socket(bind_address->ai_family, bind_address->ai_socktype,
                         bind_address->ai_protocol);
  if (!ISVALIDSOCKET(socket_listen))
  {
    fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
    return 1;
  }

  // Allow socket address reuse
  int yes = 1;
  if (setsockopt(socket_listen, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) <
      0)
  {
    fprintf(stderr, "setsockopt() failed. (%d)\n", GETSOCKETERRNO());
    return 1;
  }

  printf("Binding socket to local address...\n");
  if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen))
  {
    fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
    return 1;
  }
  freeaddrinfo(bind_address);

  printf("Listening...\n");
  if (listen(socket_listen, 10) < 0)
  {
    fprintf(stderr, "listen() failed. (%d)\n", GETSOCKETERRNO());
    return 1;
  }

  printf("Waiting for connections...\n");
  while (1)
  {
    struct sockaddr_storage client_address;
    socklen_t client_len = sizeof(client_address);
    SOCKET socket_client =
        accept(socket_listen, (struct sockaddr *)&client_address, &client_len);
    if (!ISVALIDSOCKET(socket_client))
    {
      // If we are shutting down, don't print an error
      if (socket_listen == -1)
      {
        return 0;
      }
      fprintf(stderr, "accept() failed. (%d)\n", GETSOCKETERRNO());
      return 1;
    }

    char address_buffer[100];
    getnameinfo((struct sockaddr *)&client_address, client_len, address_buffer,
                sizeof(address_buffer), 0, 0, NI_NUMERICHOST);
    printf("New connection from %s\n", address_buffer);

    int pid = fork();
    if (pid == 0)
    { // child process
      CLOSESOCKET(socket_listen);

      while (1)
      {
        char read[1024];
        int bytes_received = recv(socket_client, read, 1024, 0);
        if (bytes_received < 1)
        {
          printf("Client disconnected.\n");

          CLOSESOCKET(socket_client);
          exit(0);
        }
        int j;
        for (j = 0; j < bytes_received; ++j)
          read[j] = read[j];

        replace_char(read, '\n', '\0');
        handleClient(socket_client, read);
      }
    }
    CLOSESOCKET(socket_client);
  } // while(1)

  printf("Finished.\n");
  return 0;
}

int main()
{

  signal(SIGINT, handle_shutdown);  // Catches CTRL+C
  signal(SIGTERM, handle_shutdown); // Catches kill command
  setup(SERVER_PORT);

  return 0;
}