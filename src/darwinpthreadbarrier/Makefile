# to compile with clang, change this to 'clang' or use 'make CC=clang [...]'
CC=gcc
override CFLAGS += -O3

pthread_barrier.o : src/pthread_barrier.c src/pthread_barrier.h
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	-rm *.o
