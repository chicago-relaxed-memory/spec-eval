CC=gcc-5
ARMCC=arm-linux-gnueabi-gcc -march=armv8-a
override CFLAGS += -O3 -std=c11
LDFLAGS=-lpthread

# Change this here, or override on the command line with e.g. 'make SRC=myfile.c [...]'
SRC=relaxedmemNew4.c

EXECS=relaxedmemattack
ASMS=$(addsuffix .s, $(EXECS))

.PHONY : all asm
all : $(EXECS) $(ASMS)
asm : $(ASMS)

relaxedmemattack : $(SRC) for_0_to_2047.h
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

relaxedmemattack.s : $(SRC) for_0_to_2047.h
	$(CC) $(CFLAGS) -S $< -o $@

.PHONY : clean
clean :
	-rm -rf $(EXECS) $(ASMS)
