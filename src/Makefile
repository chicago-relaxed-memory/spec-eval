# to compile with clang, change this to 'clang' or use 'make CC=clang [...]'
CC=gcc
override CFLAGS += -O3
LDFLAGS=-lpthread -lm

EXECS=relaxedattack

.PHONY : all
all: $(EXECS) $(addsuffix .s, $(EXECS))

relaxedattack: relaxedattack.c for_0_to_2047.h
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

relaxedattack.s : relaxedattack.c for_0_to_2047.h
	$(CC) $(CFLAGS) -S $< -o $@ $(LDFLAGS)

.PHONY : run
run: relaxedattack
	./$< run 100 3  # approximately tuned well on my (Craig's) machine

.PHONY : runtune
runtune: relaxedattack
	./$< tune

.PHONY : clean
clean:
	-rm -rf $(EXECS) *.s
