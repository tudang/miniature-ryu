CFLAGS=-g
LDFLAGS=-lrt -lpthread -levent

all: netpaxosclient netpaxos 

netpaxosclient:	netpaxosclient.o config.h

netpaxos: netpaxos.o config.h

clean:
	rm -f *.o netpaxosclient netpaxos 
