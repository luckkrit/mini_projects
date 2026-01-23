#include "server.h"
#include <signal.h>
#include <sys/mman.h>
// int handleClient(Store *store, SOCKET socket_client,char *read){
//     printf("%s\n", read);
//     char *token = strtok(read," ");
//     int c=0;
//     int isBuy = 0;
//     char productId[PRODUCTID_SIZE];
//     int quantity = 0;
//     while(token!=NULL){
//         if(c==0){
//             if(strcmp(token, "BUY")==0){
//                 isBuy = 1;
//             }
//         }
//         if(c==1){
//             strcpy(productId, token);
//         }
//         if(c==2){
//             quantity = atoi(token);
//             updateStore(store, productId, quantity);
//             saveStore(store, STORE_FILENAME);
//             printStore(store);
//         }
//         c++;
//         token = strtok(NULL, " ");
//     }
//     if(isBuy==1){
//         char *buffer = "BUY 1\0";
//         send(socket_client, buffer, strlen(buffer), 0);
//     }else{
//         char *buffer = "BUY 0\0";
//         send(socket_client, buffer, strlen(buffer), 0);
//     }
//     return 0;
// }
CommandEntry commandTable[] = {
    {"BUY", handleBuy},
    {"VIEW", handleView},
    {"LOGIN", handleLogin},
    {NULL, NULL} // Sentinel to mark the end
};

void handleBuy(User *user,Store *store, SOCKET client, char *saveptr) {
  char *sessionId = strtok_r(NULL, " ", &saveptr);
  char *productId = strtok_r(NULL, " ", &saveptr);
  char *qtyStr = strtok_r(NULL, " ", &saveptr);
  char response[255];
  sessionId = atol(sessionId);
  User *luser = getUserBySession(user, sessionId);
  if(luser==NULL){
        sprintf(response, "LOGIN 2\n");
        // printf("%s %d\n",response, strlen(response));
        send(client, response, strlen(response), 0);
  }else{
    if (productId && qtyStr) {
        sprintf(response, "BUY 1\n");
        updateStore(store, productId, atoi(qtyStr));
        send(client, response, strlen(response), 0);
      } else {
        sprintf(response, "BUY 0\n");
        send(client, response, strlen(response), 0);
      }
  }

}

void handleView(User *user,Store *store, SOCKET client, char *saveptr) {
  // Logic for viewing store...
  send(client, "VIEW_SUCCESS", 12, 0);
}
void handleLogin(User *user, Store *store, SOCKET client, char *saveptr) {
  // printf("line: %s\n", saveptr);
  char *username = strtok_r(NULL, " ", &saveptr);
  char *password = strtok_r(NULL, " ", &saveptr);
  char response[255];

  // printf("username: %s\n", username);
  // printf("password: %s\n", password);

  // send(client, "LOGIN_SUCCESS", 13, 0);
  if (username && password) {
    // send(client, "LOGIN_SUCCESS", 13, 0);
    User *luser = loginUser(user, username, password);
    
    // send(client, "LOGIN_SUCCESS", 13, 0);
    if(luser!=NULL){
      sprintf(response,"LOGIN 0 %lu", luser->sessionID);
      send(client, response, strlen(response), 0);
    }else{
      sprintf(response,"LOGIN 1");
      send(client, response, strlen(response), 0);
    }
    
    // send(client, response, strlen(response), 0);
  }else{
    sprintf(response,"LOGIN 1");
    send(client, response, strlen(response), 0);
  }
  
  
}
int handleClient(User *user,Store *store, SOCKET socket_client, char *read) {
  // printf("line: %s\n", read);
  char *saveptr;
  char *commandName = strtok_r(read, " \n\r", &saveptr);
  
  if (commandName == NULL)
    return -1;

  for (int i = 0; commandTable[i].commandName != NULL; i++) {
    if (strcmp(commandName, commandTable[i].commandName) == 0) {
      // Found the command! Execute its function.
      printf("Found command: %s\n", commandName);
      commandTable[i].handler(user, store, socket_client, saveptr);
      return 0;
    }
  }

  send(socket_client, "ERROR: Unknown Command", 22, 0);
  return -1;
}
int setup(char *port) {
  printf("Configuring local address...\n");
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  // // Init shared memory
  // GameState *shared_state = mmap(NULL, sizeof(GameState), 
  //                                  PROT_READ | PROT_WRITE, 
  //                                  MAP_SHARED | MAP_ANONYMOUS, -1, 0);

  // Init store and user
  Store *store = (Store *)malloc(sizeof(Store));
  loadStore(store, STORE_FILENAME);
  User *user = (User *)calloc(1,sizeof(User));    
  loadUser(user, USER_FILENAME);

  struct addrinfo *bind_address;
  getaddrinfo(0, port, &hints, &bind_address);

  printf("Creating socket...\n");
  SOCKET socket_listen;
  socket_listen = socket(bind_address->ai_family, bind_address->ai_socktype,
                         bind_address->ai_protocol);
  if (!ISVALIDSOCKET(socket_listen)) {
    fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
    return 1;
  }

  // Allow socket address reuse
  int yes = 1;
  if (setsockopt(socket_listen, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) <
      0) {
    fprintf(stderr, "setsockopt() failed. (%d)\n", GETSOCKETERRNO());
    return 1;
  }

  printf("Binding socket to local address...\n");
  if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen)) {
    fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
    return 1;
  }
  freeaddrinfo(bind_address);

  printf("Listening...\n");
  if (listen(socket_listen, 10) < 0) {
    fprintf(stderr, "listen() failed. (%d)\n", GETSOCKETERRNO());
    return 1;
  }

  printf("Waiting for connections...\n");
  while (1) {
    struct sockaddr_storage client_address;
    socklen_t client_len = sizeof(client_address);
    SOCKET socket_client =
        accept(socket_listen, (struct sockaddr *)&client_address, &client_len);
    if (!ISVALIDSOCKET(socket_client)) {
      fprintf(stderr, "accept() failed. (%d)\n", GETSOCKETERRNO());
      return 1;
    }

    char address_buffer[100];
    getnameinfo((struct sockaddr *)&client_address, client_len, address_buffer,
                sizeof(address_buffer), 0, 0, NI_NUMERICHOST);
    printf("New connection from %s\n", address_buffer);

    int pid = fork();
    if (pid == 0) { // child process
      CLOSESOCKET(socket_listen);
      
      
      while (1) {
        char read[1024];
        int bytes_received = recv(socket_client, read, 1024, 0);
        if (bytes_received < 1) {
          printf("Client disconnected.\n");

          
          CLOSESOCKET(socket_client);
          exit(0);
        }
        int j;
        for (j = 0; j < bytes_received; ++j)
          read[j] = read[j];

        handleClient(user,store, socket_client, read);
        // send(socket_client, read, bytes_received, 0);
      }
    }
    CLOSESOCKET(socket_client);
  } // while(1)

  printf("Closing listening socket...\n");
  CLOSESOCKET(socket_listen);

  // 1. Free all Stock nodes in the linked list
  freeStore(store);

  // 2. Free the Store struct itself
  free(store);

  // 3. Free user
  freeUser(user);

  printf("Finished.\n");
  return 0;
}

int main() {
  // Store *store = (Store *)malloc(sizeof(Store));
  // loadStore(store, STORE_FILENAME);
  // printStore(store);
  // saveStore(store, "store.txt");
  signal(SIGCHLD, SIG_IGN);
  setup("3030");

  return 0;
}