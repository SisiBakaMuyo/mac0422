CFLAGS = -Wall -O2 -ansi -pedantic -Wno-unused-result -lreadline
CC = gcc
RM = rm
#---------------------------------------------------

all: ep3 clean

ep3: main.o
	$(CC) -o ep3 main.o $(CFLAGS)

main.o: ep3.c ep3.h
	$(CC) -c ep3.c $(CFLAGS)

clean:
	mkdir -p ../bin
	mv ep3 ../bin/
	rm *.o
	rm unidade
