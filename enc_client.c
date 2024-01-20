/*
    CS374 Assignment 5: One-Time Pad Encryption / Final
    Author: Maxwell Cole
    Date: December 13, 2023
    Description:
        This assignment implements a one-time pad encryption/decryption service. 
    It includes three key components: 
    1. keygen.c: a key generator which allows users to create a random key of a specified length.
    2. enc_server.c and enc_client.c: a pair of programs where the client sends plaintext and a key to the server for encryption. The server then returns the ciphertext.
    3. dec_server.c and dec_client.c: a pair of programs where the client sends ciphertext and a key to the server for decryption. The server then returns the original plaintext.
    Each component is designed to communicate via sockets in a networked environment. The encryption and decryption are based on modular arithmetic, ensuring a secure exchange of information.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUFFER_SIZE 1024

// client implementation inspired by client.c from assignment doc

// Function for handling errors: prints the error message and exits the program
void error(const char *msg) { 
    perror(msg); 
    exit(1); 
} 

// Function to initialize the client's address structure with given port number and hostname
void setupAddressStruct(struct sockaddr_in* address, int portNumber, char* hostname) {

    // Zeroing the struct to avoid garbage values
    memset((char*) address, '\0', sizeof(*address)); 

    // Setting the address family to AF_INET for IPv4
    address->sin_family = AF_INET;

    // Converting the port number from host byte order to network byte order
    address->sin_port = htons(portNumber);

    // Retrieving host information based on hostname
    struct hostent* hostInfo = gethostbyname(hostname); 
    if (hostInfo == NULL) { 
        fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
        exit(1); 
    }

    // Copying the host's address to the address structure
    // The memcpy function is used to ensure that the entire address is copied correctly.
    memcpy((char*) &address->sin_addr.s_addr, hostInfo->h_addr_list[0], hostInfo->h_length);
}

// Function to read content from a file into a buffer
void readFileContent(const char* filename, char* buffer) {

    // Opening the file for reading
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "CLIENT: ERROR opening file %s\n", filename);
        exit(1);
    }

    // Reading the content of the file into the buffer. This is necessary to send the content over the network.
    if (fgets(buffer, BUFFER_SIZE - 1, file) == NULL) {
        fprintf(stderr, "CLIENT: ERROR reading file %s\n", filename);
        fclose(file);
        exit(1);
    }

    // Removing newline character from the end of the buffer to preserve text
    buffer[strcspn(buffer, "\n")] = '\0'; // Remove newline
    fclose(file);
}

// Function to send data over a socket, returns the number of bytes sent
int sendData(int socketFD, const char* data, int dataSize) {
    int totalSent = 0;
    int charsWritten; // Bytes sent in each send call

    // Loop until all the data is sent. This is necessary because the send function might not send all data in one call.
    while (totalSent < dataSize) {

        // Sending the data, detecting errors if no data is able to be sent/written
        // The third argument is the number of bytes to send, which is the total size minus the number of bytes already sent.
        charsWritten = send(socketFD, data + totalSent, dataSize - totalSent, 0);
        if (charsWritten < 0) {
            error("CLIENT: ERROR writing to socket");
        }

        if (charsWritten == 0) {
            break; // If no data was sent, break the loop to avoid an infinite loop
        }

        totalSent += charsWritten;
    }
    return totalSent;
}

// Main function (sets up the client, connects to the server, and handles data transmission)
int main(int argc, char *argv[]) {
    int socketFD, portNumber, charsRead;
    struct sockaddr_in serverAddress;
    char plaintext[BUFFER_SIZE];
    char key[BUFFER_SIZE];

    // Check if the correct number of arguments are provided. This is necessary to ensure the program has all the information it needs to run.
    if (argc < 4) { 
        fprintf(stderr,"USAGE: %s plaintext key port\n", argv[0]); 
        exit(1); 
    } 

    // Reading plaintext and key from files
    readFileContent(argv[1], plaintext);
    readFileContent(argv[2], key);

    // Validating key length against plaintext length
    if (strlen(key) < strlen(plaintext)) {
        fprintf(stderr, "CLIENT: ERROR, key length is shorter than plaintext\n");
        exit(1);
    }

    // Set up the server address struct. This is necessary to establish a connection to the server. 
    setupAddressStruct(&serverAddress, atoi(argv[3]), "localhost");

    // Creating a streaming socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0); 
    if (socketFD < 0) {
        error("CLIENT: ERROR opening socket");
    }

    // Connect to server. This is necessary to start communication with the server
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        error("CLIENT: ERROR connecting");
    }

    // Send data to the server. This is necessary to transmit the plaintext and key to the server.
    // The sendData function returns the number of bytes sent, which can be used to check if all data was sent successfully.

    // Sending the size of the plaintext
    int plaintextSize = strlen(plaintext);
    if (sendData(socketFD, (char*)&plaintextSize, sizeof(plaintextSize)) < sizeof(plaintextSize)) {
        fprintf(stderr, "CLIENT: WARNING: Not all plaintext size data written to socket!\n");
    }

    // Sending the plaintext itself
    if (sendData(socketFD, plaintext, plaintextSize) < plaintextSize) {
        fprintf(stderr, "CLIENT: WARNING: Not all plaintext data written to socket!\n");
    }

    // Sending the size of the key 
    int keySize = strlen(key);
    if (sendData(socketFD, (char*)&keySize, sizeof(keySize)) < sizeof(keySize)) {
        fprintf(stderr, "CLIENT: WARNING: Not all key size data written to socket!\n");
    }

    // Sending the key itself
    if (sendData(socketFD, key, keySize) < keySize) {
        fprintf(stderr, "CLIENT: WARNING: Not all key data written to socket!\n");
    }

    // Clearing the plaintext buffer and receive encrypted text from the server
    memset(plaintext, '\0', BUFFER_SIZE);
    charsRead = recv(socketFD, plaintext, BUFFER_SIZE - 1, 0); 
    if (charsRead < 0) {
        error("CLIENT: ERROR reading from socket");
    }

    // Outputting the received encrypted text
    printf("%s\n", plaintext);

    // Closing the socket and exit the program to ensure no dangling sockets
    close(socketFD); 
    return 0;
}