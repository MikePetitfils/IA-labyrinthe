all: labclient

labclient: main.o
	gcc -o labclient main.o

main.o: main.c main.h
	gcc -c main.c -W -Wall

clean:
	rm -rf *.o

mrproper: clean
	rm -rf labclient
