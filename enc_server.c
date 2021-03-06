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
bool DEBUG = false;

/* DATA PROTOTYPES */
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
void encrypt(encrypt_data_t *data);
char convert(int);
int  deconvert(char);

int main(int argc, char *argv[])
{
    int newCon;
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t sizeOfClientInfo = sizeof(clientAddress);
    pid_t con_pid;
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
        newCon = accept(listenSocket, (struct sockaddr *)&clientAddress, &sizeOfClientInfo);

        if (newCon < 0)
        {
            error("ERROR on accept");
        }

        if(DEBUG) { 
            printf("SERVER: Connected to client running at host %d port %d\n",
               ntohs(clientAddress.sin_addr.s_addr),
               ntohs(clientAddress.sin_port));
        }

        /* FORK: 
           forking the main process to allow for multiple children in a 
           concurrent manner
        */
        if ((con_pid = fork()) == 0)
        {
            
            // close initial listening socket
            close(listenSocket);

            /* 
            AUTH SECTION
            */
            connection_data.auth_len = recv(newCon, connection_data.auth, sizeof(AUTHLEN) - 1, 0);

            if (strcmp(connection_data.auth, "ENC") != 0)
            {
                char response[] = "INVALID";
                write(newCon, response, sizeof(response));
                exit(2);
            }
            else
            {
                // write confirmation back to client
                char response[] = "ENC";
                write(newCon, response, sizeof(response));
            }

            /* 
            DATA GET SECTION
            */
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

            /* 
            KEY GET SECTION
            */
            memset(connection_data.key, '\0', MAXLEN);
            connection_data.key_len_read = recv(newCon, connection_data.key, MAXLEN, 0);

            if (connection_data.key_len_read < 0)
            {
                error("ERROR reading from socket");
            }

            // Encrypt message
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

/*
 * FUNCTION - init_data
 * DESC - initializes the required data members
 *        for the struct
 *
 */
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

/*
 * FUNCTION - encrypt
 * DESC - one time pads the data recieved by the client
 *        and populates the data struct's cipher based on 
 *        that key/data pair
 */
void encrypt(encrypt_data_t *connection_data)
{

    int cipher = 0;
    int data = 0;
    int key = 0;
    if (DEBUG)
    {
        printf("SERVER: Recieved encrypted data from client: \"%s\"\n", connection_data->data);
    }
    if (DEBUG)
    {
        printf("SERVER: Recieved key from client: \"%s\"\n", connection_data->key);
    }

    for (int i = 0; i <= connection_data->data_len_read; i++)
    {
        if (connection_data->data[i] == 0)
        {
            continue;
        }

        data = deconvert(connection_data->data[i]);
        key = deconvert(connection_data->key[i]);

        cipher = (data + key) % 27;

        connection_data->cipher[i] = convert(cipher);
    }

    if (DEBUG)
    {
        printf("\nSERVER: Decrypted text sent to client: \"%s\"\n", connection_data->cipher);
    }

    return;
}

/*
 * FUNCTION - convert 
 * DESC - converts an int ascii value to an index
 *        corresponding to the specific element in the
 *        allowable chars 
*/
char convert(int input)
{
    char allowablechars[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
    return allowablechars[input];
}

/*
 * FUNCTION - deconvert 
 * DESC - deconverts a char to an ascii int value to 
 *        corresponding to the specific element in the
 *        allowable chars 
*/
int deconvert(char input)
{
    int result = -1;
    char allowablechars[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

    for (int i = 0; i < strlen(allowablechars); i++)
    {
        if (allowablechars[i] == input)
        {
            result = i;
            break;
        }
    }

    return result;
}
