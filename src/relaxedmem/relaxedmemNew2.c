#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>  // aligned_alloc()
#include <string.h>  // memset()
#include <emmintrin.h>
#include "cache_sets.h"
#include "for_0_to_2047.h"

static int x;
static int y;
static int z;
static uintptr_t w;

static unsigned alwaysTrue = 0;
static volatile int b = 1;

#define NUM_HITS_DESIRED 6  // hits in L2.  Along the way, many more guesses will match 12 bits but not 16 and we'll evict from L1.
#define CACHELINE_SIZE 64
#define NUM_CACHELINES_TO_TRY (NUM_HITS_DESIRED * (1 << 16) / CACHELINE_SIZE)
static uint8_t array[NUM_CACHELINES_TO_TRY * CACHELINE_SIZE];

#ifndef SECRET
#error Please define SECRET when compiling (e.g. pass -DSECRET=123 to the compiler)
#endif

static void* threadfunc(void* dummy) {
    x = 1;

#define TRY_CACHELINE(i) array[i*CACHELINE_SIZE] = 123;
#define TRY_CACHELINE_SECOND(i) array[(i+2048)*CACHELINE_SIZE] = 123;
#define TRY_CACHELINE_THIRD(i) array[(i+4096)*CACHELINE_SIZE] = 123;

    if(!alwaysTrue) { x = SECRET; }
    else { x = 1; }

    /*
    for(unsigned i = 0; i < NUM_CACHELINES_TO_TRY; i++) {
      array[i*CACHELINE_SIZE] = 123;
    }
    */
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

    _mm_clflush(&y);
    _mm_clflush(&x);
	y = x;
    _mm_clflush(&y);
    _mm_clflush(&x);

    /*
    // note that NUM_CACHELINES_TO_TRY currently resolves to 6144
    FOR_0_TO_2047(TRY_CACHELINE)
    FOR_0_TO_2047(TRY_CACHELINE_SECOND)
    FOR_0_TO_2047(TRY_CACHELINE_THIRD)
    */

	pthread_join(thread, &dummy);
    w = (uintptr_t)array[0];  // keep gcc from optimizing away array entirely?
	return (z > 1);
}

int main() {
/*
    aligned_array = (uint8_t*)aligned_alloc(1<<12, NUM_ADDRESSES*(1<<12));
    if(aligned_array == NULL) {
      fprintf(stderr, "Error: aligned_alloc returned NULL\n");
      exit(1);
    }
*/

    memset(array, 0, NUM_CACHELINES_TO_TRY * CACHELINE_SIZE);

	unsigned count = 0;
	for(int i = 0; i < 100000; i++) {
		alwaysTrue++;
		if(attack()) count++;
	}
	printf("%u %llu\n", count, w);
	return 0;
}
