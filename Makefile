CC=gcc
NUM_TESTS=1 2 3 4

build: dccthread tests


tests:
	$(foreach var,$(NUM_TESTS),$(CC) test$(var).c dccthread.o -o test$(var) -Wall -lrt;)

dccthread:
	$(CC) -c dccthread.c -o dccthread.o -Wall -lrt

clean:
	rm -f *~
	rm -rf *.o
	rm -f main
	$(foreach var,$(NUM_TESTS),rm -f test$(var);)
