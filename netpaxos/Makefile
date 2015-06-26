all: test sender server receiver storage validate

test:	test.o
	gcc -o test -lrt -lpthread test.c

sender:	sender.o
	gcc -o sender -lrt -lpthread sender.c

server: server.o
	gcc -o server -lrt -lpthread server.c

receiver: receiver.o
	gcc -o receiver -lrt -lpthread receiver.c

storage: storage.o
	gcc -o storage -lrt storage.c

validate: validate.o
	gcc -o validate validate.c
clean:
	rm -f *.o sender server storage receiver test validate
