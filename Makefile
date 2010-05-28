all: labclient

labclient: main.o
	gcc -o labclient main.o

main.o: main.c main.h
	gcc -o main.o -c main.c -W -Wall -ansi -pedantic

clean:
	rm -rf *.o

mrproper: clean
	rm -rf labclient
