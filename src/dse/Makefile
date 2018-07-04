# to compile with clang, change this to 'clang' or use 'make CC=clang [...]'
CC=gcc
override CFLAGS += -O3
LDFLAGS=-lpthread -lm

EXECS=dseattack

.PHONY : all
all: $(EXECS) $(addsuffix .s, $(EXECS))

dseattack: dseattack.c for_0_to_2047.h
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

dseattack.s : dseattack.c for_0_to_2047.h
	$(CC) $(CFLAGS) -S $< -o $@ $(LDFLAGS)

.PHONY : run
run: $(EXECS)
	./$< run 100 4  # approximately tuned well on my (Craig's) machine

.PHONY : runtune
runtune: $(EXECS)
	./$< tune

.PHONY : clean
clean:
	-rm -rf $(EXECS) *.s
