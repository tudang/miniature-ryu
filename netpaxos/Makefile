CC=gcc
CFLAGS=-I.
LIBS=-lrt -levent
DEPS = config.h
OBJ = netpaxos.o netpaxoslearner.o netpaxosclient.c 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(LIBS)

netpaxos: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)
clean:
	rm -f netpaxos *.o
