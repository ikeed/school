CFLAGS=-Wall

client:	client.cpp sox.o
	g++ ${CFLAGS} -o client client.cpp sox.o
server:	server.cpp sox.o
	g++ ${CFLAGS} -o server server.cpp sox.o
sox.o:	sox.cpp sox.h
	g++ ${CFLAGS} -c sox.cpp
clean:
	rm -f client server *.o

