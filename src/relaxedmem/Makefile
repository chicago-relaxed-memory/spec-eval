CC=gcc
CFLAGS=-O3
LDFLAGS=-lpthread

# Override this with e.g. 'make SRC=myfile.c [...]'
SRC=relaxedmem.c

EXECS=matchguess wrongguess

.PHONY : all asm
all : $(EXECS) asm
asm : $(addsuffix .s, $(EXECS))

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
	-rm -rf $(EXECS) *.s
