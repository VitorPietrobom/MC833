CC = gcc
CFLAGS = -Wall -g

all: server client

server: server.o cJSON.o
	$(CC) $(CFLAGS) -o server server.o cJSON.o

client: client.o cJSON.o
	$(CC) $(CFLAGS) -o client client.o cJSON.o

server.o: server.c
	$(CC) $(CFLAGS) -c server.c

client.o: client.c
	$(CC) $(CFLAGS) -c client.c

cJSON.o: cJSON.c
	$(CC) $(CFLAGS) -c cJSON.c cJSON.h


clean:
	rm -f *.o server client