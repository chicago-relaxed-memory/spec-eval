#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>

static int x;
static int y;
static int z;

static unsigned alwaysTrue = 0;
static int b = 1;

#define SECRET 234987

// create a bunch of register pressure, and make sure to depend on the passed
// value
static unsigned createRegisterPressure(int);

static void* threadfunc(void* dummy) {
    uintptr_t ret;
    do {
      if(!alwaysTrue) { x = SECRET; }
      else { x = 1; }
      ret = createRegisterPressure(x);
      do {
        if(y == 0) { z = 111; }
      } while (x != 1);
    } while (b == 0 && (b = 1));  // side effect never fires, but compiler doesn't know that
    b = 8;
	return (void*) ret;
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

static volatile unsigned v1 = 2, v2 = 30, v3 = 88, v4 = 134,
                         v5 = 123, v6 = 23456, v7 = 987, v8 = 2938375;

static unsigned createRegisterPressure(int dependOn) {
  unsigned a1 = dependOn + v1;
  unsigned a2 = dependOn + v2;
  unsigned a3 = a1 * v3 - a2 + v1;
  unsigned a4 = a2 / a3 + v4 - v3 / a1;
  unsigned a5 = a1 - v5 + a2 + v4 + v3 / a4 - a3;
  unsigned a6 = a3 * a4 * a5 - a2 - a1;
  unsigned a7 = a5 - a4 * v2 + a6 + a3 / a1;
  unsigned a8 = a6 + v6 - a2 / a7 + a5 + a4 + a3;
  unsigned a9 = a7 * a8 - v7 * a1 + a2 * a3 * a4;
  unsigned a10 = a7 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
  unsigned a11 = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
  return a10 - a9 * a8 + a7 - a6 * a5 + a4 * a3 / a2 + a1;
}
