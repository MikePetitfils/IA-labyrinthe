all: labclient

labclient: main.o
	gcc `pkg-config --cflags --libs glib-2.0` -o labclient main.o

main.o: main.c main.h
	gcc  `pkg-config --cflags --libs glib-2.0` -c main.c -W -Wall

clean:
	rm -rf *.o

mrproper: clean
	rm -rf labclient
