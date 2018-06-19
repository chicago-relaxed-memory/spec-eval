#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>  // usleep()

static volatile bool alwaysFalse = false;
static unsigned x, y;

#ifndef SECRET
#error Please pass either -DSECRET=true or -DSECRET=false when compiling.
#endif

// in the SECRET=false case, gcc feels the need to keep the x=1 store intact.
// in the SECRET=true case, gcc is fine skipping the dead x=1 store and doing just the x=2 store.
static void* threadfunc(void* dummy) {
  x = 1;
  if(alwaysFalse) {
    if(SECRET) y = 1;
  } else {
    y = 1;
  }

  // waste some time, but don't use a syscall like usleep().
  // If we use a syscall, gcc wants to do the x=1 store first, regardless
  volatile int v = 0;
  for(int i = 0; i < 5000000; i++) v++;

  x = 2;
  return (void*) (uintptr_t) y;  // keep gcc from removing the y=1 store entirely
}

static int attack() {
  x = 0;
  y = 0;
  int a, b;
  void* dummy;
  pthread_t thread;
  pthread_create(&thread, NULL, &threadfunc, NULL);
  usleep(1);
  a = x;
  b = y;
  pthread_join(thread, &dummy);
  return a;
}

int main() {
  unsigned count0 = 0, count1 = 0, count2 = 0, countOther = 0;
  for(int i = 0; i < 1000; i++) {
    switch(attack()) {
      case 0: count0++; break;
      case 1: count1++; break;
      case 2: count2++; break;
      default: countOther++; break;
    }
  }
  printf("0: %u  1: %u  2: %u  other: %u\n", count0, count1, count2, countOther);
  return 0;
}
