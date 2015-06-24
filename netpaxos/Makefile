test:	test.o
	gcc -o test -lrt -lpthread test.c

client:	client.o
	gcc -o client -lrt -lpthread client.c

server: server.o
	gcc -o server -lrt -lpthread server.c

single: single.o
	gcc -o single -lrt -lpthread single.c

storage: storage.o
	gcc -o storage -lrt storage.c

validate: validate.o
	gcc -o validate validate.c
clean:
	rm -f *.o client server storage
