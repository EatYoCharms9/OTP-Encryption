OTP ENCYPTION README

*NOTE: A makefile is provided
To compile the program, utilize the line:   make
*NOTE: program will not compile unless compiled in C99.

To run the program after compilation, utilize the following in Bash: 
./keygen [keyname] > [exported plaintext file with key]
enc_server [port number] &
dec_server [port number] &

enc_client [plaintext to be converted] [key for conversion] [port number] > [ciphertext file for export]
dec_client [ciphertext to be converted] [key for conversion] [port number] > [plaintext file for export]

*NOTE: ensure background processes are killed after execution