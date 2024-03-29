CC=gcc
CFLAGS=-ansi -Wall -pedantic -O2
EP1SHELL=newsh.c leitorSring.o
EP1SIMPROCESS = ep1.c leitorArquivo.o pilha.o fila.o processo.h

all: shell simprocess clean

shell: $(EP1SHELL)
	$(CC) $(EP1SHELL) $(CFLAGS) -lreadline -ltermcap -o shell

simprocess: $(EP1SIM)
	$(CC) $(EP1SIMPROCESS) $(CFLAGS) -lpthread -o simprocess

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf *.o
