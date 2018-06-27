#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>

static int x;
static int y;
static int z;

// 'volatile' ensures compiler keeps the check
volatile bool alwaysFalse = false;

#ifndef SECRET
#error Please define SECRET when compiling (e.g. pass -DSECRET=123 to the compiler)
#endif

static void* threadfunc(void* dummy) {
	intptr_t c = 0;
	if(y == 0) { x = 1; }
	else if(alwaysFalse) { x = SECRET; c++; }
	else { x = 1; z = 1; }
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
	return (z == 1);
}

int main() {
	unsigned count = 0;
	for(int i = 0; i < 100000; i++) {
		if(attack()) count++;
	}
	printf("%u\n", count);
	return 0;
}
