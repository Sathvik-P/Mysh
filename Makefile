all:	mysh

mysh:	mysh.o
	@gcc -o -Wall -O3 -o mysh mysh.o

mysh.o:	mysh.c
	@cc -c mysh.c

clean:	
	@rm -f mysh mysh.o
