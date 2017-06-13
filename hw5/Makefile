all: client server

client: client.c
	gcc -Wall -Werror -DDEBUG -g -o client client.c

server: server.c
	gcc -Wall -Werror -DDEBUG -g -o server server.c -pthread

clean:
	rm -f *~ *.o client server
