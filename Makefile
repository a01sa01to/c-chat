all: server client

server: server.c myutil.h io.h sock.h
	gcc -o server server.c -lpthread -D_GNU_SOURCE

client: client.c myutil.h io.h sock.h
	gcc -o client client.c -lpthread -D_GNU_SOURCE

run_server: server
	./server 50000

run_client: client
	./client localhost 50000

clean:
	rm -f server client
