#include "server.h"
#include <signal.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>    /* For O_CREAT, O_RDWR constants */
#include <sys/stat.h> /* For mode constants like 0644 */


CommandEntry commandTable[] = {
    {"BUY", handleBuy},
    {"VIEW", handleView},
    {"LOGIN", handleLogin},
    {NULL, NULL} // Sentinel to mark the end
};

SOCKET socket_listen = -1;
Store *store = NULL;
User *user = NULL;
sem_t *sem = NULL;

void handleBuy(User *user,Store *store, SOCKET client, char *saveptr) {
  char *sessionId = strtok_r(NULL, " ", &saveptr);
  char *productId = strtok_r(NULL, " ", &saveptr);
  char *qtyStr = strtok_r(NULL, " ", &saveptr);
  char response[255];
  char *endptr;
  long unsigned sessionId2 = strtoul(sessionId,endptr, 10); // base 10
  if (*endptr != '\0' && *endptr != '\n' && *endptr != '\r') {
    // This catches if the string contained letters or symbols
    printf("Warning: Partial conversion. Ended at: %s\n", endptr);
  }
  printf("SessionId: %s %lu\n",sessionId, sessionId2);
  User *luser = getUserBySession(user, sessionId2);
  if(luser==NULL){
        sprintf(response, "BUY 2\n");
        // printf("%s %d\n",response, strlen(response));
        send(client, response, strlen(response), 0);
  }else{
    if (productId && qtyStr) {
        // 1. Wait for the lock (Block if another process is buying)
        sem_wait(sem);
        sprintf(response, "BUY 0\n");

        // 2. CRITICAL SECTION: Only one process can be here at a time
        updateStore(store, productId, atoi(qtyStr));
        saveStore(store, STORE_FILENAME);
        // 3. Release the lock
        sem_post(sem);
        
        send(client, response, strlen(response), 0);
      } else {
        sprintf(response, "BUY 1\n");
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

void handle_shutdown(int sig){
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

int setup(char *port) {
  printf("Configuring local address...\n");
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  
  // init semaphore
  sem = sem_open("/store_lock", O_CREAT, 0644, 1);

  // Init store and user using shared memory (mmap)
  store = mmap(NULL, sizeof(Store), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1 ,0);
  loadStore(store, STORE_FILENAME);  
  user = mmap(NULL, sizeof(User), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1 ,0);
  loadUser(user, USER_FILENAME);

  struct addrinfo *bind_address;
  getaddrinfo(0, port, &hints, &bind_address);

  printf("Creating socket...\n");
  
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
      // If we are shutting down, don't print an error
        if (socket_listen == -1) { 
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

  
  printf("Finished.\n");
  return 0;
}

int main() {
   
  signal(SIGINT, handle_shutdown);  // Catches CTRL+C
  signal(SIGTERM, handle_shutdown); // Catches kill command
  setup("3030");

  return 0;
}