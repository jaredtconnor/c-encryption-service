#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

#define MAXLEN 100000

void encrypt()
{

  return;
}

void sig_handler(int sig)
{

  pid_t PID;
  int status;

  do
  {
    PID = waitpid(-1, &status, WNOHANG);

  } while (PID != -1);
}

// Error function used for reporting issues
void error(const char *msg)
{
  perror(msg);
  exit(1);
}

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in *address,
                        int portNumber)
{

  // Clear out the address struct
  memset((char *)address, '\0', sizeof(*address));

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);
  // Allow a client at any address to connect to this server
  address->sin_addr.s_addr = INADDR_ANY;
}

int main(int argc, char *argv[])
{
  int connectionSocket, charsRead;
  char buffer[MAXLEN];
  struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo = sizeof(clientAddress);

  signal(SIGCHLD, sig_handler);
  pid_t con_pid;

  // Check usage & args
  if (argc < 2)
  {
    fprintf(stderr, "USAGE: %s port\n", argv[0]);
    exit(1);
  }

  // Create the socket that will listen for connections
  int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (listenSocket < 0)
  {
    error("ERROR opening socket");
  }

  // Set up the address struct for the server socket
  setupAddressStruct(&serverAddress, atoi(argv[1]));

  // Associate the socket to the port
  if (bind(listenSocket,
           (struct sockaddr *)&serverAddress,
           sizeof(serverAddress)) < 0)
  {
    error("ERROR on binding");
  }

  // Start listening for connetions. Allow up to 5 connections to queue up
  listen(listenSocket, 5);

  // Accept a connection, blocking if one is not available until one connects
  while (1)
  {
    // Accept the connection request which creates a connection socket
    connectionSocket = accept(listenSocket, (struct sockaddr *)&clientAddress, &sizeOfClientInfo);

    printf("SERVER: Connected to client running at host %d port %d\n", ntohs(clientAddress.sin_addr.s_addr), ntohs(clientAddress.sin_port));

    // Get the message from the client and display it
    memset(buffer, '\0', MAXLEN);

    if (connectionSocket < 0)
    {
      error("ERROR on accept");
    }

    con_pid = fork();

    // If we're child - we can read write on client via client_sockfd
    if (con_pid == -1)
    {
      close(connectionSocket);
      continue;
    } 

    else if (con_pid > 0){ 
      close(connectionSocket); 
      continue;
    }

    // Read the client's message from the socket
    charsRead = recv(connectionSocket, buffer, MAXLEN - 1, 0);

    if (charsRead < 0)
    {
      error("ERROR reading from socket");
    }

    printf("SERVER: I received this from the client: \"%s\"\n", buffer);

    // Send a Success message back to the client
    charsRead = send(connectionSocket,
                      "I am the server, and I got your message", 39, 0);
    if (charsRead < 0)
    {
      error("ERROR writing to socket");
    }

    memset(buffer, 0, MAXLEN);
    close(connectionSocket);
  }

  close(listenSocket);
  return 0;
}
