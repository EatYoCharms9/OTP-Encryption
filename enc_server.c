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
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_SIZE 1024

// Function for handling errors: prints the error message and exits the program
void error(const char *msg) {
    perror(msg);
    exit(1);
}

// Function to initialize the server's address structure with given port number
void setupAddressStruct(struct sockaddr_in* address, int portNumber){

    memset((char*) address, '\0', sizeof(*address)); // Zeroing the struct before populating, this avoids garbage values

    address->sin_family = AF_INET;  // Set address family to AF_INET for IPv4
    address->sin_port = htons(portNumber); // Convert the port number from host byte order to network byte order
    address->sin_addr.s_addr = INADDR_ANY; // Allow server to bind to any available interface
}

// Helper function to convert a character to its corresponding index
int charToIndex(char c) {
    if (c == ' ') return 26; // ' ' (space) is represented as 26; otherwise, subtract 'A' to get index (0-25) (outlined in example)
    return c - 'A';
}

// Helper function to convert an index back to a character
char indexToChar(int i) {
    if (i == 26) return ' '; // 26 is represented as ' '; otherwise, add 'A' to index to get character
    return 'A' + i;
}

// Function to encrypt the given text using the given key
void encrypt(char *text, const char *key, int length) {
    for (int i = 0; i < length; i++) {

        // Converting both text and key characters to indices and perform modular addition to encrypt
        int textIdx = charToIndex(text[i]);
        int keyIdx = charToIndex(key[i]);

        // Converting the result back to a character and storing in text format for future use
        text[i] = indexToChar((textIdx + keyIdx) % 27);
    }
}

// Function to handle client-server communication process (implementation inspired by server.c from assignment doc)
void handleCommunication(int connectionSocket) {

    char buffer[BUFFER_SIZE]; // Buffer to store received plaintext
    int charsRead; // Variable for keeping track of the number of read characters

    // Initializing buffer to null characters, this ensures no garbage data
    memset(buffer, '\0', BUFFER_SIZE);

    // Performing various required checks below to ensure encryption may occur. These verify exactly how much data to expect/send.

    // Read the size of the ciphertext from the socket. This is necessary to know how much data to receive next.
    int textLength;
    charsRead = recv(connectionSocket, (char*)&textLength, sizeof(textLength), 0); 
    if (charsRead < 0){
        error("ERROR reading from socket");
    }

    // Read the ciphertext from the socket. This is necessary to get the data that needs to be decrypted.
    charsRead = recv(connectionSocket, buffer, textLength, 0); 
    if (charsRead < 0){
        error("ERROR reading from socket");
    }

    // Read the size of the key from the socket. This is necessary to know how much data to receive next.
    int keyLength;
    charsRead = recv(connectionSocket, (char*)&keyLength, sizeof(keyLength), 0); 
    if (charsRead < 0){
        error("ERROR reading from socket");
    }

    // Initialize a buffer to hold the key.
    char key[BUFFER_SIZE];

    // Read the key from the socket. This is necessary to get the key that will be used to encrypt the ciphertext.
    charsRead = recv(connectionSocket, key, keyLength, 0);
    if (charsRead < 0){
        error("ERROR reading from socket");
    }

    // Encrypt by calling the encrypt function once all checks have passed. utilize the received plaintext and key
    encrypt(buffer, key, textLength);

    // Send the encrypted text back to the client. This is necessary to provide the client with the result of the encryption.
    charsRead = send(connectionSocket, buffer, textLength, 0); 
    if (charsRead < 0){
        error("ERROR writing to socket");
    }
}

// Main function (sets up the server socket and handles incoming connections)
int main(int argc, char *argv[]){
    int connectionSocket;
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t sizeOfClientInfo = sizeof(clientAddress);
    pid_t pid;

    // Check if the correct number of arguments are provided. This is necessary to ensure the program has all the information it needs to run.
    if (argc < 2) { 
        fprintf(stderr,"USAGE: %s port\n", argv[0]); 
        exit(1);
    } 

    // Set up the socket. This is necessary to listen for incoming connections.
    int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket < 0) {
        error("ERROR opening socket");
    }

    // Initializing the server address structure
    setupAddressStruct(&serverAddress, atoi(argv[1]));

    // Binding server's address and port to the listening socket
    if (bind(listenSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0){
        error("ERROR on binding");
    }

    // Listen on the socket for incoming connections, allow a queue of up to 5 pending connections
    listen(listenSocket, 5); 

    // Main server loop. This loop runs indefinitely, waiting for connections and handling them.
    while(1){

        // Accept a connection, blocking if one is not available until one connects. This is necessary to establish a connection with a client.
        connectionSocket = accept(listenSocket, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); 
        if (connectionSocket < 0){
            error("ERROR on accept");
        }

        // Fork a new process to handle the connection. This is necessary to allow the server to handle multiple connections simultaneously.
        pid = fork();
        if (pid < 0) {
            error("ERROR on fork");
        }

        if (pid == 0) { // Child process
            close(listenSocket); // Close the listening socket in the child process
            handleCommunication(connectionSocket); // Handle communication with the client
            
            exit(0); // Exit the child process after handling
        } else { // Otherwise the process is a parent process

            close(connectionSocket); // Close the connection socket in the parent process (assignment parameters)
        }
    }

    // Close listening sockets / return to ensure program exit (nohang)
    close(listenSocket); 
    return 0;
}