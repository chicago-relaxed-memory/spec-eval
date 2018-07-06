# to compile with clang, change this to 'clang' or use 'make CC=clang [...]'
CC=gcc
override CFLAGS += -O3
LDFLAGS=-lpthread -lm

EXECS=dseattack

.PHONY : all
all: $(EXECS) $(addsuffix .s, $(EXECS))

dseattack.o: dseattack.c for_0_to_2047.h
	$(CC) $(CFLAGS) -c $< -o $@

dseattack: dseattack.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

dseattack.s : dseattack.c for_0_to_2047.h
	$(CC) $(CFLAGS) -S $< -o $@

.PHONY : run
run: $(EXECS)
	./$< run 100 2  # approximately tuned well on my (Craig's) machine
# with clang, the first parameter is ignored, the second one seems
#   to need to be 4 for actual 100% accuracy.  3 is very close.

.PHONY : runtune
runtune: $(EXECS)
	./$< tune

.PHONY : clean
clean:
	-rm $(EXECS) *.s *.o
