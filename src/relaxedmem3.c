#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/types.h>

static volatile int y;
static int z;

// 'volatile' ensures compiler keeps the check
volatile bool alwaysFalse = false;

#ifndef SECRET
#error Please define SECRET when compiling (e.g. pass -DSECRET=123 to the compiler)
#endif

static void* threadfunc(void* arg) {
	int* xstar = (int*) arg;
	intptr_t c = 0;
	for(int i = 0; i < 1000; i++) {
		if(alwaysFalse) { *xstar = SECRET; c++; }
		else if(y == 0) { *xstar = 1; }
		else { *xstar = 1; z = 1; }
	}
	return (void*) c;
}

static bool attack() {
	int x = 0;
	y = 0;
	z = 0;
	void* dummy;
	pthread_t thread;
	pthread_create(&thread, NULL, &threadfunc, &x);
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


