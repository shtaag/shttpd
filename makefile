all: shttpd
	
shttpd: shttpd.o filereader.o
	gcc -o shttpd -lm shttpd.o filereader.o

filereader.o: filereader.c filereader.h
	gcc -o filereader.o -c $(CFLAGS) filereader.c

shttpd.o: shttpd.c filereader.h
	gcc -o shttpd.o -c $(CFLAGS) shttpd.c
