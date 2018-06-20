CC=gcc
override CFLAGS += -O3
LDFLAGS=-lpthread -lm

EXECS=relaxedattack_false relaxedattack_true

.PHONY : all
all: $(EXECS) $(addsuffix .s, $(EXECS))

relaxedattack_%: relaxedattack.c
	$(CC) $(CFLAGS) -DSECRET=$(basename $*) $^ -o $@ $(LDFLAGS)

%.s : override CFLAGS += -S

.PHONY : runtrue
runtrue: relaxedattack_true
	./$< 100 200000  # approximately tuned well on my (Craig's) machine

.PHONY : runfalse
runfalse: relaxedattack_false
	./$< 100 200000  # approximately tuned well on my (Craig's) machine

.PHONY : runtune
runtune: relaxedattack_false
	./$< tune

.PHONY : clean
clean:
	-rm -rf $(EXECS) *.s
