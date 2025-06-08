FLAGS = -Wall -g -O0 -c

ep3:  ep3.o
		gcc ep3.o -o ep3 -lreadline

ep3.o: ep3.c ep3.h
		gcc $(FLAGS) ep3.c

clean:
	mkdir -p ../bin
	mv ep3 ../bin/
	rm *.o

