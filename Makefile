all: server client

server: server.c
	gcc -o server server.c

client: client.c
	gcc -o client client.c

run_server: server
	./server 50000

run_client: client
	./client localhost 50000

clean:
	rm -f server client
