CC=gcc
CFLAGS=-o
EP1SHELL= newsh.c 
EP1SIM = ep1.c 

all: shell ep1 clean

shell: $(EP1SHELL)
	$(CC) $(EP1SHELL) $(CFLAGS) shell -lreadline -ltermcap

ep1: $(EP1SIM)
	$(CC) $(EP1SIM) $(CFLAGS) ep1 -lpthread 

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf *.o
