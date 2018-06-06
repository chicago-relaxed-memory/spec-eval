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

static void* threadfunc(void* dummy) {
    do {
      if(!alwaysTrue) { x = SECRET; }
      else { x = 1; }
      do {
        if(y == 0) { z = 111; }
      } while (x != 1);
    } while (b == 0 && (b = 1));  // side effect never fires, but compiler doesn't know that
    b = 8;
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
