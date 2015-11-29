CC=gcc
NUM_TESTS=1

build: dccthread tests


tests:
	$(foreach var,$(NUM_TESTS),$(CC) test$(var).c dccthread.o -o test$(var) -Wall;)

dccthread:
	$(CC) -c dccthread.c -o dccthread.o -Wall

run:
	./main

clean:
	rm -f *~
	rm -rf *.o
	rm -f main
	$(foreach var,$(NUM_TESTS),rm test$(var);)
