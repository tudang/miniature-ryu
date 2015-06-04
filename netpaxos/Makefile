client:	client.o
	gcc -o client -lrt client.c

nouvo: threading.o
	gcc -o nouvo -lrt -lpthread threading.c

server: server.o
	gcc -o server -lrt -lpthread server.c

storage: storage.o
	gcc -o storage -lrt storage.c

clean:
	rm -f *.o client server
