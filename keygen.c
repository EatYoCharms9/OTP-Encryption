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
#include <time.h>

// Defining a constant character set for key generation (defined by assignment parameters)
const char CHAR_SET[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

// Main function (controls simple key generation)
int main(int argc, char *argv[]) {

    // Checking for correct number of command-line arguments
    if (argc != 2) {
        fprintf(stderr, "Usage: %s keylength\n", argv[0]);
        return 1;
    }

    // Converting the key length argument from string to integer
    int keyLength = atoi(argv[1]);

    // Validating that the key length is a positive integer
    if (keyLength <= 0) {
        fprintf(stderr, "Key length must be a positive integer\n");
        return 1;
    }

    // Seeding the random number generator
    // Assignment mentions that rand() is okay to use as assignment does not have to be cryptographically secure
    srand(time(NULL));

    // Generating and printing the key character by character
    for (int i = 0; i < keyLength; i++) {
        // Selecting a random character from the character set, then outputting it
        putchar(CHAR_SET[rand() % (sizeof(CHAR_SET) - 1)]);
    }

    // Print newline character at the end of the key for proper formatting
    putchar('\n');

    return 0;
}