CFLAGS=-Wall -g

build:	client server

client:	client.cpp sox.o mystring.o
	g++ ${CFLAGS} -o $@ $^
server:	server.cpp sox.o mystring.o
	g++ ${CFLAGS} -o $@ $^
sox.o:	sox.cpp sox.h
	g++ ${CFLAGS} -c sox.cpp
mystring.o:	mystring.cpp mystring.h
	g++ ${CFLAGS} -c mystring.cpp
clean:
	rm -f client server *.o

