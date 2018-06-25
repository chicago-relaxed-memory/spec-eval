CC=gcc
CXX=g++
override CFLAGS += -O3
LDFLAGS=-lpthread -lm

EXECS=relaxedattack

.PHONY : all
all: $(EXECS) $(addsuffix .s, $(EXECS))

relaxedattack: relaxedattack.cpp
	$(CXX) $(CFLAGS) $^ -o $@ $(LDFLAGS)

relaxedattack.s : relaxedattack.cpp
	$(CXX) $(CFLAGS) -S $^ -o $@ $(LDFLAGS)

.PHONY : run
run: relaxedattack
	./$< run 500 2  # approximately tuned well on my (Craig's) machine

.PHONY : runtune
runtune: relaxedattack
	./$< tune

.PHONY : clean
clean:
	-rm -rf $(EXECS) *.s
