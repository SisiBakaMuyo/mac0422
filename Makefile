CC=gcc
CFLAGS=-o
EP1SHELL=newsh.c 
EP1SIMPROCESS = ep1.c 

all: shell simprocess clean

shell: $(EP1SHELL)
	$(CC) $(EP1SHELL) $(CFLAGS) -lreadline -ltermcap -o shell

simprocess: $(EP1SIM)
	$(CC) $(EP1SIMPROCESS) $(CFLAGS) -lpthread -o simprocess

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf *.o
