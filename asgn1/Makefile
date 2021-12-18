httpserver : httpserver.o
	gcc -o httpserver httpserver.o

httpserver.o : httpserver.c
	gcc -c -std=c99 -Wall httpserver.c

clean :
		rm -f httpserver.c