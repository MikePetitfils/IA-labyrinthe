all : server client

server : utilities.o server.o 
	cc utilities.o server.o -o server

utilities.o : utilities.c utilities.h
	cc -c utilities.c -o utilities.o 

server.o : server.c server.h
	cc -c server.c -o server.o 
	
client : client.c
	cc client.c -o client

clean :
	rm -rf *.o

