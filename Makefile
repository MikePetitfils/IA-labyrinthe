all: labclient

labclient: main.o libbox.o
	gcc `pkg-config --cflags --libs glib-2.0` -o labclient main.o libbox.o

main.o: main.c main.h libbox.h
	gcc  `pkg-config --cflags --libs glib-2.0` -c main.c -W -Wall

libbox.o: libbox.c libbox.h
	gcc  `pkg-config --cflags --libs glib-2.0` -c libbox.c -W -Wall
clean:
	rm -rf *.o

mrproper: clean
	rm -rf labclient
