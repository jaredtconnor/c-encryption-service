#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAXLEN 100000

/* Function prototypes */
void error(const char *);
void setupAddressStruct(struct sockaddr_in *, int);

int main(int argc, char *argv[])
{
  int newCon, charsRead, charSent;
  char buffer[MAXLEN];
  struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo = sizeof(clientAddress);
  pid_t con_pid;
  static int counter = 0;

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
    newCon = accept(listenSocket,
                    (struct sockaddr *)&clientAddress,
                    &sizeOfClientInfo);
    if (newCon < 0)
    {
      error("ERROR on accept");
    }

    printf("SERVER: Connected to client running at host %d port %d\n",
           ntohs(clientAddress.sin_addr.s_addr),
           ntohs(clientAddress.sin_port));

    if ((con_pid = fork()) == -1)
    {
      close(newCon);
      continue;
    }
    else if (con_pid > 0)
    {
      close(newCon);
      counter++;
      continue;
    }
    else if (con_pid == 0)
    {

      counter++;

      memset(buffer, '\0', MAXLEN);
      charsRead = recv(newCon, buffer, MAXLEN, 0);

      if (charsRead < 0)
      {
        error("ERROR reading from socket");
      }

      printf("SERVER: Recieved this from client: \"%s\"\n", buffer);

      // Send a Success message back to the client
      charSent = send(newCon, "I am the server, and I got your message", 39, 0);

      if (charSent < 0)
      {
        error("ERROR writing to socket");
      }

      close(newCon);
    }
  }

  close(listenSocket);
  return 0;
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