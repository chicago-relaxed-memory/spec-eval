CC=gcc
override CFLAGS += -O3
LDFLAGS=-lpthread

EXECS=relaxedattack_false relaxedattack_true

.PHONY : all
all: $(EXECS) $(addsuffix .s, $(EXECS))

relaxedattack_%: relaxedattack.c
	$(CC) $(CFLAGS) -DSECRET=$(basename $*) $^ -o $@ $(LDFLAGS)

%.s : override CFLAGS += -S

.PHONY : runtrue
runtrue: relaxedattack_true
	./$^

.PHONY : runfalse
runfalse: relaxedattack_false
	./$^

.PHONY : clean
clean:
	-rm -rf $(EXECS) *.s
