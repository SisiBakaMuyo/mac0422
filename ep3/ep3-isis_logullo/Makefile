CC=gcc
CFLAGS=-o
EP3=ep3.c

all: ep3 

ep3: $(EP3)
	$(CC) $(EP3) $(CFLAGS) ep3 

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf *.o && rm -rf ep3
