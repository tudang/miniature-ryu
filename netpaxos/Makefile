all: client server storage

client:	client.o config.h
	gcc -o client -lrt -lpthread client.c

server: server.o config.h
	gcc -o server -lrt -lpthread server.c

storage: storage.o config.h
	gcc -o storage -lrt storage.c

clean:
	rm -f *.o client server storage
