CFLAGS=-g
LDFLAGS=-lrt -lpthread

all: client server storage

client:	client.o config.h

server: server.o config.h

storage: storage.o config.h

clean:
	rm -f *.o client server storage
