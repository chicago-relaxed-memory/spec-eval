#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>  // aligned_alloc()
#include "cache_sets.h"

static int x;
static int y;
static int z;
static uintptr_t w;

static unsigned alwaysTrue = 0;
static volatile int b = 1;
static uint8_t* restrict aligned_array;

#ifndef SECRET
#error Please define SECRET when compiling (e.g. pass -DSECRET=123 to the compiler)
#endif

static void* threadfunc(void* dummy) {
    x = 1;

    /*
    int* x_ptr = &x;
    w = (uintptr_t)x_ptr;
    */

    if(!alwaysTrue) { x = SECRET; }
    else { x = 1; }
    accessNCachelinesMatchingBottom12Bits(&x, aligned_array, 1);
    if(y == 0) { z = 111; }
	return 0;
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
    w = (uintptr_t)aligned_array[0];  // keep gcc from optimizing away aligned_array entirely?
	return (z > 1);
}

int main() {
    aligned_array = (uint8_t*)aligned_alloc(1<<12, NUM_ADDRESSES*(1<<12));
    if(aligned_array == NULL) {
      fprintf(stderr, "Error: aligned_alloc returned NULL\n");
      exit(1);
    }

	unsigned count = 0;
	for(int i = 0; i < 100000; i++) {
		alwaysTrue++;
		if(attack()) count++;
	}
	printf("%u %llu\n", count, w);
	return 0;
}
