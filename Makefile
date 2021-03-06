target:
	@echo "Compiling..."
	gcc -std=gnu99 -Wall -g -o enc_server enc_server.c
	gcc -std=gnu99 -Wall -g -o enc_client enc_client.c
	gcc -std=gnu99 -Wall -g -o dec_server dec_server.c
	gcc -std=gnu99 -Wall -g -o dec_client dec_client.c
	gcc -std=gnu99 -Wall -g -o keygen keygen.c
	rm -rf *.o

keygen: 
	gcc -std=gnu99 -Wall -g -o keygen keygen.c

test_key: 
	./keygen 37 > testkey

client: 
	gcc -std=gnu99 -Wall -g -o enc_client enc_client.c

client_run: 
	./enc_client test_scripts/plaintext1 testkey 56111 > testciphertext

client_test: 
	./enc_client test_scripts/plaintext1 testkey 56111

decclient: 
	gcc -std=gnu99 -Wall -g -o dec_client dec_client.c

decclient_run: 
	./dec_client testciphertext testkey 56112 > testfinalresults

server: 
	gcc -std=gnu99 -Wall -g -o enc_server enc_server.c

server_run: 
	./enc_server 56111

server_run_bg: 
	./enc_server 56111 &

server_kill: 
	pkill -f enc_server

decserver: 
	gcc -std=gnu99 -Wall -g -o dec_server dec_server.c

decserver_run: 
	./dec_server 56112

decserver_run_bg: 
	./dec_server 56112 &

decserver_kill: 
	pkill -f dec_server

clean:
	rm -rf *.o
	rm enc_server 
	rm enc_client
	rm dec_server
	rm dec_client
	rm keygen 

test:
	./p5testscript 56111 56112 > mytestresults 2>&1

valgrind-key: 
	valgrind -s --leak-check=yes --track-origins=yes --show-reachable=yes --log-file="valgrind.txt" ./keygen 25
