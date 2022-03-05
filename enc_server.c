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
  char cipher[MAXLEN];
  int cipher_len;

  int len_sent;
} encrypt_data_t;

/* Function prototypes */
void error(const char *);
void setupAddressStruct(struct sockaddr_in *, int);
void init_data(encrypt_data_t * data);
void encrypt(encrypt_data_t * data);

int main(int argc, char *argv[])
{
  int newCon, charsRead, charSent;
  char buffer[MAXLEN];
  struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo = sizeof(clientAddress);
  pid_t con_pid;
  static int counter = 0;
  encrypt_data_t connection_data;
  init_data(&connection_data);

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
  /* 
  Source - https://www.cs.dartmouth.edu/~campbell/cs50/socketprogramming.html
  */
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

    if ((con_pid = fork()) == 0)
    {

      close(listenSocket);

      printf("CHILD - Encrypting message\n");
      counter++;

      memset(connection_data.data, '\0', MAXLEN);
      connection_data.data_len_read = recv(newCon, connection_data.data, MAXLEN, 0);

      if (connection_data.data_len_read < 0)
      {
        error("ERROR reading from socket");
      }

      // Send a Success message back to the client
      char sendkey_reply[] = "SENDKEY\0";
      connection_data.len_sent = send(newCon, sendkey_reply, strlen(sendkey_reply), 0);

      if (connection_data.len_sent < 0)
      {
        error("ERROR writing to socket");
      }

      memset(connection_data.key, '\0', MAXLEN);
      connection_data.key_len_read = recv(newCon, connection_data.key, MAXLEN, 0);

      if (connection_data.key_len_read < 0)
      {
        error("ERROR reading from socket");
      }

      encrypt(&connection_data);

      // Send encrypted data to client
      connection_data.len_sent = send(newCon, connection_data.cipher, strlen(connection_data.cipher), 0);

      if (connection_data.len_sent < 0)
      {
        error("ERROR writing to socket");
      }

      close(newCon);
      exit(0); // child terminates
    }


  close(newCon); // parnet closes connected socket
  }


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

void init_data(encrypt_data_t * data)
{

  memset(data->data, '\0', MAXLEN);
  memset(data->key, '\0', MAXLEN);
  memset(data->cipher, '\0', MAXLEN);

  data->data_len_read = 0;
  data->key_len_read = 0;
  data->cipher_len = 0;
  data->len_sent = 0;
}

void encrypt(encrypt_data_t * connection_data)
{

  printf("SERVER: Recieved data from client: \"%s\"\n", connection_data->data);
  printf("SERVER: Recieved key from client: \"%s\"\n", connection_data->key);
  printf("Pre - Dec cipher data: %d\n", connection_data->data[3]);
  printf("Pre - Char cipher data: %c\n", connection_data->data[3]);
  printf("Pre - Dec key data: %d\n", connection_data->key[3]);
  printf("Pre - Char key data: %c\n", connection_data->key[3]);

  for (int i = 0; i <= connection_data->key_len_read; i++)
  {
    connection_data->cipher[i] = (connection_data->data[i] + connection_data->key[i]) % 26 + 65;
  }

  printf("Post - Dec key data: %d\n", connection_data->cipher[3]);
  printf("Post - Char key data: %d\n", connection_data->cipher[3]);
  printf("SERVER: Cipher text sent to client: \"%s\"\n", connection_data->key);

  return;
}