# to compile with clang, change this to 'clang' or use 'make CC=clang [...]'
CC=gcc
override CFLAGS += -O3 -I../darwinpthreadbarrier/src
LDFLAGS=-lpthread -lm

EXECS=dseattack

.PHONY : all
all: $(EXECS) $(addsuffix .s, $(EXECS))

# This Makefile considers pthread_barrier.o to be "always out of date", so that it
# always invokes the Makefile in that directory which actually updates it if necessary
.PHONY: ../darwinpthreadbarrier/pthread_barrier.o
../darwinpthreadbarrier/pthread_barrier.o:
	$(MAKE) -C ../darwinpthreadbarrier

dseattack.o: dseattack.c for_0_to_2047.h
	$(CC) $(CFLAGS) -c $< -o $@

dseattack: dseattack.o ../darwinpthreadbarrier/pthread_barrier.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

dseattack.s : dseattack.c for_0_to_2047.h
	$(CC) $(CFLAGS) -S $< -o $@

.PHONY : run
run: $(EXECS)
	./$< run 100 4  # approximately tuned well on my (Craig's) machine

.PHONY : runtune
runtune: $(EXECS)
	./$< tune

.PHONY : clean
clean:
	-$(MAKE) -C ../darwinpthreadbarrier clean
	-rm $(EXECS) *.s *.o
