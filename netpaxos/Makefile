client:	client.o
	gcc -o client -lrt client.c

server: server.o
	gcc -o server -lrt -lpthread server.c

clean:
	rm -f *.o client server
