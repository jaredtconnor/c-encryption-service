#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAXLEN 100000

typedef struct encrypt_data
{ 
  char data[MAXLEN]; 
  int data_len_read;
  char key[MAXLEN];
  int key_len_read;
  int len_sent;
} encrypt_data_t;


/* Function prototypes */
void error(const char *);
void setupAddressStruct(struct sockaddr_in *, int);
void init_data(encrypt_data_t data);
void encrypt(encrypt_data_t data);

int main(int argc, char *argv[])
{
  int newCon, charsRead, charSent;
  char buffer[MAXLEN];
  struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo = sizeof(clientAddress);
  pid_t con_pid;
  static int counter = 0;
  encrypt_data_t connection_data;
  init_data(connection_data); 

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

      memset(connection_data.data, '\0', MAXLEN);
      connection_data.data_len_read = recv(newCon, connection_data.data, MAXLEN, 0);

      if (connection_data.data_len_read < 0)
      {
        error("ERROR reading from socket");
      }
      printf("SERVER: Recieved data from client: \"%s\"\n", connection_data.data);

      // Send a Success message back to the client
      char data_reply_2[] = "SENDKEY\0";
      connection_data.len_sent = send(newCon, data_reply_2, strlen(data_reply_2), 0);

      if (connection_data.len_sent < 0)
      {
        error("ERROR writing to socket");
      }

      memset(connection_data.key, '\0', MAXLEN);
      connection_data.key_len_read = recv(newCon, connection_data.key, MAXLEN, 0);

      printf("SERVER: Recieved key from client: \"%s\"\n", connection_data.key);
      if (connection_data.key_len_read < 0)
      {
        error("ERROR reading from socket");
      }

      // Send a Success message back to the client
      char data_reply_1[] = "Data recieved\0";
      connection_data.len_sent = send(newCon, data_reply_1, strlen(data_reply_1), 0);

      if (connection_data.len_sent < 0)
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

void init_data(encrypt_data_t data){ 

  memset(data.data, '\0', MAXLEN);
  memset(data.key, '\0', MAXLEN);

  data.data_len_read = 0;
  data.key_len_read = 0;
  data.len_sent = 0;

}

void encrypt(encrypt_data_t data){ 











}