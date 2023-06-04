all: server client

server: server.c common/* server_h/*
	gcc -o server server.c -lpthread -D_GNU_SOURCE

client: client.c common/* client_h/*
	gcc -o client client.c -lpthread -D_GNU_SOURCE

run_server: server
	./server 50000

run_client: client
	./client localhost 50000

clean:
	rm -f server client
