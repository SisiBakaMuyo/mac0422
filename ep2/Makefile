CC=gcc
CFLAGS=-o
EP2=ep2.c
PTHREAD=-lpthread

all: ep2 

ep2: $(EP2)
	$(CC) $(EP2) $(CFLAGS) ep2 $(PTHREAD) 

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf *.o && rm -rf ep2
