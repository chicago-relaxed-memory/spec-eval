CC=gcc
ARMCC=arm-linux-gnueabi-gcc -march=armv8-a
override CFLAGS += -O3 -std=c11 -mrtm
LDFLAGS=-lpthread

# Change this here, or override on the command line with e.g. 'make SRC=myfile.c [...]'
SRC=relaxedmemNew3.c

EXECS=matchguess wrongguess
ASMS=$(addsuffix .s, $(EXECS))

.PHONY : all asm
all : $(EXECS) $(ASMS)
asm : $(ASMS)

matchguess : $(SRC) cache_sets.h for_0_to_2047.h
	$(CC) $(CFLAGS) -DSECRET=1 $< -o $@ $(LDFLAGS)

wrongguess : $(SRC) cache_sets.h for_0_to_2047.h
	$(CC) $(CFLAGS) -DSECRET=234987 $< -o $@ $(LDFLAGS)

matchguess.s : $(SRC) cache_sets.h for_0_to_2047.h
	$(CC) $(CFLAGS) -S -DSECRET=1 $< -o $@

wrongguess.s : $(SRC) cache_sets.h for_0_to_2047.h
	$(CC) $(CFLAGS) -S -DSECRET=234987 $< -o $@

.PHONY : clean
clean :
	-rm -rf $(EXECS) $(ASMS)
