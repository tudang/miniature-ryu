CFLAGS=-g
LDFLAGS=-lrt -lpthread -levent

all: netpaxos 

netpaxos: netpaxos.o netpaxosclient.o netpaxoslearner.o config.h

clean:
	rm -f *.o netpaxos 
