all: cracker

cracker: sha256.o code.o reverse.o
	  gcc -o sha256.o code.o reverse.o 

sha256.o: sha.c sha256.h
	  gcc -o sha256.o -c sha256.c -W -Wall -ansi -pedantic 

code.o: code.c reverse.h
	  gcc -o code.o -pthread -c code.c -lpthread -W -Wall -ansi -pedantic

reverse.o: reverse.c sha256.h reverse.h
   	  gcc -o reverse.o -c reverse.c -W -Wall -ansi -pedantic 

clean:
	rm -rf *.o

mrproper: clean
	rm -rf cracker




