CC=gcc
override CFLAGS += -O3
LDFLAGS=-lpthread -lm

EXECS=relaxedattack_0 relaxedattack_1

.PHONY : all
all: $(EXECS) $(addsuffix .s, $(EXECS))

relaxedattack_%: relaxedattack.c
	$(CC) $(CFLAGS) -DSECRET=$(basename $*) $^ -o $@ $(LDFLAGS)

%.s : override CFLAGS += -S

.PHONY : run0
run0: relaxedattack_0
	./$< 100 200000  # approximately tuned well on my (Craig's) machine

.PHONY : run1
run1: relaxedattack_1
	./$< 100 200000  # approximately tuned well on my (Craig's) machine

.PHONY : runtune
runtune: relaxedattack_0
	./$< tune

.PHONY : clean
clean:
	-rm -rf $(EXECS) *.s
