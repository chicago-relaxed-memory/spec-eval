# to compile with clang (or a custom-built gcc, or anything), change this here or use 'make CC=somecc [...]'
CC=gcc
ARMCC=arm-linux-gnueabi-gcc -march=armv8-a
override CFLAGS += -O3 -std=c11
LDFLAGS=-lpthread

SRC=lsrattack.c

EXECS=lsrattack
ASMS=$(addsuffix .s, $(EXECS))

.PHONY : all asm
all : $(EXECS) $(ASMS)
asm : $(ASMS)

lsrattack : $(SRC) for_0_to_2047.h
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

lsrattack.s : $(SRC) for_0_to_2047.h
	$(CC) $(CFLAGS) -S $< -o $@

.PHONY : run
run : $(EXECS)
	./$< run 3

.PHONY : runtune
runtune: $(EXECS)
	./$< tune

.PHONY : clean
clean :
	-rm -rf $(EXECS) $(ASMS)
