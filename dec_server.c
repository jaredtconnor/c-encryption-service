#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>

#define MAXLEN 100000
#define AUTHLEN 50

typedef struct encrypt_data
{
    char auth[AUTHLEN];
    int auth_len;
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
void init_data(encrypt_data_t *data);
void decrypt(encrypt_data_t *data);
char convert(int);
int deconvert(char);

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

            connection_data.auth_len = recv(newCon, connection_data.auth, sizeof(AUTHLEN) - 1, 0);
            printf("AUTH KEY - %s\n", connection_data.auth);

            if (strcmp(connection_data.auth, "DEC") != 0)
            {
                char response[] = "INVALID";
                write(newCon, response, sizeof(response));
                exit(2);
            }
            else
            {
                // write confirmation back to client
                char response[] = "DEC";
                write(newCon, response, sizeof(response));
            }

            printf("CHILD - Encrypting message\n");
            counter++;

            memset(connection_data.cipher, '\0', MAXLEN);
            connection_data.cipher_len = recv(newCon, connection_data.cipher, MAXLEN, 0);

            if (connection_data.cipher_len < 0)
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

            connection_data.len_sent = 0;
            memset(connection_data.key, '\0', MAXLEN);
            connection_data.key_len_read = recv(newCon, connection_data.key, MAXLEN, 0);

            if (connection_data.key_len_read < 0)
            {
                error("ERROR reading from socket");
            }

            decrypt(&connection_data);

            // Send encrypted data to client
            connection_data.len_sent = send(newCon, connection_data.data, strlen(connection_data.data), 0);

            if (connection_data.len_sent < 0)
            {
                error("ERROR writing to socket");
            }

            connection_data.len_sent = 0;
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

void init_data(encrypt_data_t *data)
{

    memset(data->auth, '\0', AUTHLEN);
    memset(data->data, '\0', MAXLEN);
    memset(data->key, '\0', MAXLEN);
    memset(data->cipher, '\0', MAXLEN);

    data->auth_len = 0;
    data->data_len_read = 0;
    data->key_len_read = 0;
    data->cipher_len = 0;
    data->len_sent = 0;
}

void decrypt(encrypt_data_t *connection_data)
{

    int decipher = 0;
    int data = 0;
    int key = 0;
    printf("SERVER: Recieved encrypted data from client: \"%s\"\n", connection_data->cipher);
    printf("SERVER: Recieved key from client: \"%s\"\n", connection_data->key);

    for (int i = 0; i < connection_data->cipher_len; i++)
    {
        if (connection_data->cipher[i] == 0)
        {
            continue;
        }

        data = deconvert(connection_data->cipher[i]);
        key = deconvert(connection_data->key[i]);

        decipher = (data - key) % 27;

        if (decipher < 0)
        {
            decipher = decipher + 27;
        }

        connection_data->data[i] = convert(decipher);
    }

    printf("\nSERVER: Decrypted text sent to client: \"%s\"\n", connection_data->data);

    return;
}

char convert(int input)
{
    char allowablechars[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
    return allowablechars[input];
}

int deconvert(char input)
{
    int result = -1;
    char allowablechars[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

    for (int i = 0; i < 27; i++)
    {
        if (allowablechars[i] == input)
        {
            result = i;
            break;
        }
    }

    return result;
}
