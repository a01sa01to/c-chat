all: server client

server: server.c myutil.h ds.h io.h
	gcc -o server server.c

client: client.c myutil.h ds.h io.h
	gcc -o client client.c

run_server: server
	./server 50000

run_client: client
	./client localhost 50000

clean:
	rm -f server client
