#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>

static int x;
static int y;
static int z;

static unsigned alwaysTrue = 1;

#define SECRET 1

static void* threadfunc(void* dummy) {
	intptr_t c = 0;
    for(volatile int i = 0; i < 1; i++) {
      if(y == 0) { x = 1; }
      else if(!alwaysTrue) { x = SECRET; c++; }
      else { z = 111; x = 1; }
    }
	return (void*) c;
}

static bool attack() {
	x = 0;
	y = 0;
	z = 0;
	void* dummy;
	pthread_t thread;
	pthread_create(&thread, NULL, &threadfunc, NULL);
	y = x;
	pthread_join(thread, &dummy);
	return (z > 1);
}

int main() {
	unsigned count = 0;
	for(int i = 0; i < 100000; i++) {
		alwaysTrue++;
		if(attack()) count++;
	}
	printf("%u\n", count);
	return 0;
}
