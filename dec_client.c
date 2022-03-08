#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()
#include <stdbool.h>
#include <ctype.h>

#define MAXLEN 100000
#define AUTHLEN 50

bool DEBUG = false;

// Error function used for reporting issues
void error(const char *msg)
{
    perror(msg);
    exit(0);
}

size_t checkfilelen(FILE *inputfile)
{

    ssize_t pos = ftell(inputfile);
    fseek(inputfile, 0, SEEK_END);
    ssize_t length = ftell(inputfile);
    fseek(inputfile, pos, SEEK_SET);

    return length;
}

// Set up the address struct
void setupAddressStruct(struct sockaddr_in *address,
                        int portNumber,
                        char *hostname)
{

    // Clear out the address struct
    memset((char *)address, '\0', sizeof(*address));

    // The address should be network capable
    address->sin_family = AF_INET;
    // Store the port number
    address->sin_port = htons(portNumber);

    // Get the DNS entry for this host name
    struct hostent *hostInfo = gethostbyname(hostname);
    if (hostInfo == NULL)
    {
        fprintf(stderr, "CLIENT: ERROR, no such host\n");
        exit(0);
    }
    // Copy the first IP address from the DNS entry to sin_addr.s_addr
    memcpy((char *)&address->sin_addr.s_addr,
           hostInfo->h_addr_list[0],
           hostInfo->h_length);
}

int main(int argc, char *argv[])
{
    int socketFD, charsWritten, charsRead, authWritten;
    struct sockaddr_in serverAddress;
    char auth[MAXLEN] = "DEC";
    char buffer[MAXLEN];
    char reply_buffer[MAXLEN];
    char key_buffer[MAXLEN];
    char decrypyt_buffer[MAXLEN];
    FILE *inputfile;
    FILE *keyfile;
    size_t input_len;
    size_t key_len;
    // Check usage & args
    if (argc < 4)
    {
        fprintf(stderr, "USAGE: %s inputfile keyfile port\n", argv[0]);
        exit(0);
    }

    // Setup input file to read and key
    inputfile = fopen(argv[1], "r");
    keyfile = fopen(argv[2], "r");

    // Clear out the buffer array
    memset(buffer, '\0', sizeof(buffer));
    memset(key_buffer, '\0', sizeof(key_buffer));

    // READ INPUT
    if (inputfile != NULL)
    {
        input_len = fread(buffer, sizeof(char), MAXLEN, inputfile);

        if (ferror(inputfile) != 0)
        {
            error("Unable to read file\n");
        }

        buffer[strcspn(buffer, "\r\n")] = 0;
        fclose(inputfile);
    }

    // READ KEY
    if (keyfile != NULL)
    {
        key_len = fread(key_buffer, sizeof(char), MAXLEN, keyfile);

        if (ferror(keyfile) != 0)
        {
            error("Unable to read key file\n");
        }

        buffer[strcspn(buffer, "\r")] = 0;
        fclose(keyfile);
    }

    // DATA ERROR - input len is longer than available key
    if (key_len < input_len)
    {
        error("CLIENT: IS THIS FUCKING WORKING ERROR Keygen is unable to cover input length\n");
    }

    // Create a socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0)
    {
        error("CLIENT: ERROR opening socket");
    }

    // Set up the server address struct
    setupAddressStruct(&serverAddress, atoi(argv[3]), "localhost");

    // Connect to server
    if (connect(socketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        error("CLIENT: ERROR connecting");
    }

    /*
    AUTH SECTION
    */
    authWritten = send(socketFD, auth, strlen(auth), 0);
    if (authWritten < strlen(auth))
    {
        error("CLIENT: WARNING: Not all data written to socket!\n");
    }
    charsRead = recv(socketFD, reply_buffer, sizeof(reply_buffer) - 1, 0);

    if (strcmp(reply_buffer, "DEC") != 0)
    {
        error("SERVER: Unable to contact dec server\n");
        exit(2);
    }
    /*
    DATA SEND SECTION
    */
    charsWritten = send(socketFD, buffer, strlen(buffer), 0);

    if (charsWritten < 0)
    {
        error("CLIENT: ERROR writing to socket");
    }

    if (charsWritten < strlen(buffer))
    {
        error("CLIENT: WARNING: Not all data written to socket!\n");
    }

    /*
    KEY SEND SECTION
    */
    memset(reply_buffer, '\0', sizeof(reply_buffer));
    charsRead = recv(socketFD, reply_buffer, sizeof(reply_buffer) - 1, 0);
    if (charsRead < 0)
    {
        error("CLIENT: ERROR reading from socket");
    }

    if (strcmp(reply_buffer, "SENDKEY\0") == 0)
    {
        // Write to the server
        charsWritten = send(socketFD, key_buffer, strlen(key_buffer), 0);
        // printf("Requesting Key\n");
    }

    memset(decrypyt_buffer, '\0', sizeof(decrypyt_buffer));

    charsRead = recv(socketFD, decrypyt_buffer, sizeof(decrypyt_buffer) - 1, 0);
    if (charsRead < 0)
    {
        error("CLIENT: ERROR reading from socket");
    }

    if (DEBUG)
    {
        printf("CLIENT: I received this encyrpted message the server: \"%s\"\n", decrypyt_buffer);
    }

    fprintf(stdout, "%s", decrypyt_buffer);

    // Close the socket
    close(socketFD);
    return 0;
}