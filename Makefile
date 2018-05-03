CC = gcc
CFLAGS = -g -Wall
INCLUDE = -Ilibfractal/ -I$(HOME)/local/include -I$(HOME)/usr/lib
LIB = -lpthread -lSDL -L/usr/include/SDL -L$(HOME)/local/lib

all: main lib
	$(CC) $(CFLAGS) -o main main.o libfractal/libfractal.a $(LIB)

main: main.c
	$(CC) $(CFLAGS) -c main.c $(INCLUDE)

lib:
	make -C libfractal/
	
clean:
	rm *.o main
	
cleanLib:
	make clean -C libfractal/
