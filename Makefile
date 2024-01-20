all: enc_server enc_client dec_server dec_client keygen

enc_server:
	gcc -std=c99 -o enc_server enc_server.c

enc_client:
	gcc -std=c99 -o enc_client enc_client.c

dec_server:
	gcc -std=c99 -o dec_server dec_server.c

dec_client:
	gcc -std=c99 -o dec_client dec_client.c

keygen:
	gcc -std=c99 -o keygen keygen.c

clean:
	rm -f enc_server enc_client dec_server dec_client keygen	