dog : dog.o
	gcc -o dog dog.o

dog.o : dog.c
	gcc -c -std=c99 -Wall dog.c

clean :
		rm -f dog.c