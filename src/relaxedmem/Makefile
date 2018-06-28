CC=gcc
ARMCC=arm-linux-gnueabi-gcc -march=armv8-a
override CFLAGS += -O3
LDFLAGS=-lpthread

# Change this here, or override on the command line with e.g. 'make SRC=myfile.c [...]'
SRC=relaxedmem16.c

EXECS=matchguess wrongguess
ASMS=$(addsuffix .s, $(EXECS))

.PHONY : all asm
all : $(EXECS) $(ASMS)
asm : $(ASMS)

matchguess : $(SRC)
	$(CC) $(CFLAGS) -DSECRET=1 $< -o $@ $(LDFLAGS)

wrongguess : $(SRC)
	$(CC) $(CFLAGS) -DSECRET=234987 $< -o $@ $(LDFLAGS)

matchguess.s : $(SRC)
	$(CC) $(CFLAGS) -S -DSECRET=1 $< -o $@

wrongguess.s : $(SRC)
	$(CC) $(CFLAGS) -S -DSECRET=234987 $< -o $@

.PHONY : clean
clean :
	-rm -rf $(EXECS) $(ASMS)
