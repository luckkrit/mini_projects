#include "server.h"

#include "server.h"
#include <signal.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>    /* For O_CREAT, O_RDWR constants */
#include <sys/stat.h> /* For mode constants like 0644 */

SOCKET socket_listen = -1;
League *league = NULL;
sem_t *sem = NULL;

void handleAddBoard(CommandContext *ctx)
{
  char *teamTitle = strtok_r(NULL, COMMAND_SEPARATOR, &ctx->rawInput);

  char response[MESSAGE_SIZE];

  if (isEmptyOrSpace(teamTitle))
  {
    snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_ADD_BOARD, COMMAND_SEPARATOR, STATUS_INVALID_ARGUMENTS);
    send(ctx->clientSocket, response, strlen(response), 0);
  }
  else
  {
    // 1. Wait for the lock (Block if another process is buying)
    sem_wait(sem);

    // 2. CRITICAL SECTION: Only one process can be here at a time
    int result = addTeam(ctx->league, teamTitle);
    saveAllTeams(ctx->league, TEAM_FILENAME);
    // 3. Release the lock
    sem_post(sem);
    if (result == 0)
    {
      snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_ADD_BOARD, COMMAND_SEPARATOR, STATUS_OK);
      send(ctx->clientSocket, response, strlen(response), 0);
    }
    else
    {
      snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_ADD_BOARD, COMMAND_SEPARATOR, STATUS_FAIL);
      send(ctx->clientSocket, response, strlen(response), 0);
    }
  }
}

void handleAddTask(CommandContext *ctx)
{
    char *teamId = strtok_r(NULL, COMMAND_SEPARATOR, &ctx->rawInput);
    char *taskTitle = strtok_r(NULL, COMMAND_SEPARATOR, &ctx->rawInput);

    char response[MESSAGE_SIZE];

    if (teamId == NULL || taskTitle == NULL ||
        isEmptyOrSpace(teamId) || isEmptyOrSpace(taskTitle))
    {
        snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_ADD_TASK, COMMAND_SEPARATOR, STATUS_INVALID_ARGUMENTS);
        send(ctx->clientSocket, response, strlen(response), 0);
        return;
    }

    Team *team = hasTeam(ctx->league, teamId);

    if (team == NULL)
    {
        snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_ADD_TASK, COMMAND_SEPARATOR, STATUS_BOARD_NOT_FOUND);
        send(ctx->clientSocket, response, strlen(response), 0);
        return;
    }

    // 1. Wait for the lock
    sem_wait(sem);

    // 2. CRITICAL SECTION
    int result = addPlayer(team, taskTitle);
    saveAllPlayers(ctx->league, PLAYER_FILENAME);

    // 3. Release the lock
    sem_post(sem);

    if (result == 0)
    {
        snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_ADD_TASK, COMMAND_SEPARATOR, STATUS_OK);
    }
    else
    {
        snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_ADD_TASK, COMMAND_SEPARATOR, STATUS_FAIL);
    }
    send(ctx->clientSocket, response, strlen(response), 0);
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
      .league = league,
      .clientSocket = socket_client,
      .rawInput = saveptr,
  };

  CommandEntry commandTable[] = {
      {COMMAND_ADD_BOARD, handleAddBoard},
      {COMMAND_ADD_TASK, handleAddTask},
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

  // Init league using shared memory (mmap)
  league = mmap(NULL, sizeof(League), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  loadModel(league, LEAGUE_FILENAME, TEAM_FILENAME, PLAYER_FILENAME);

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
  printf("Binding to port: %s\n", SERVER_PORT);
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

        printf("%s\n", read);
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