#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>

static int x;
static int y;
static volatile int z;

// 'volatile' ensures compiler keeps the check
volatile bool alwaysFalse = false;

#ifndef SECRET
#error Please define SECRET when compiling (e.g. pass -DSECRET=123 to the compiler)
#endif

static void* threadfunc(void* dummy) {
  // Apparently gcc will move a read past 31 writes but no more!
  x = 1;
  if (alwaysFalse) { x = SECRET; }
  if (alwaysFalse) { x = SECRET; }
  if (alwaysFalse) { x = SECRET; }
  if (alwaysFalse) { x = SECRET; }
  if (alwaysFalse) { x = SECRET; }
  if (alwaysFalse) { x = SECRET; }
  if (alwaysFalse) { x = SECRET; }
  if (alwaysFalse) { x = SECRET; }
  if (alwaysFalse) { x = SECRET; }
  if (alwaysFalse) { x = SECRET; }
  if (alwaysFalse) { x = SECRET; }
  if (alwaysFalse) { x = SECRET; }
  if (alwaysFalse) { x = SECRET; }
  if (alwaysFalse) { x = SECRET; }
  if (alwaysFalse) { x = SECRET; }
  if (alwaysFalse) { x = SECRET; }
  if (alwaysFalse) { x = SECRET; }
  if (alwaysFalse) { x = SECRET; }
  if (alwaysFalse) { x = SECRET; }
  if (alwaysFalse) { x = SECRET; }
  if (alwaysFalse) { x = SECRET; }
  if (alwaysFalse) { x = SECRET; }
  if (alwaysFalse) { x = SECRET; }
  if (alwaysFalse) { x = SECRET; }
  if (alwaysFalse) { x = SECRET; }
  if (alwaysFalse) { x = SECRET; }
  if (alwaysFalse) { x = SECRET; }
  if (alwaysFalse) { x = SECRET; }
  if (alwaysFalse) { x = SECRET; }
  if (alwaysFalse) { x = SECRET; }
  // In the case that SECRET is 1, gcc will remove all of the if statements
  // and move the read of y all the way to the top of the function, so the
  // read of y is guaranteed to happen before the write of x. In that case,
  // we're guaranteed to get z = 2. Otherwise the read of y won't be moved
  // so we have a possibility of seeing y = 1, and hence writing z = 1.
  if (y) { z = 1; } else { z = 2; }
  return NULL;
}

static bool attack() {
  // We need volatile aliases for x and y to make sure the writes actually happen.
  volatile int* volatile_x = &x;
  volatile int* volatile_y = &y;
  x = 0;
  y = 0;
  z = 0;
  void* dummy;
  pthread_t thread;
  // Fire up the worker
  pthread_create(&thread, NULL, &threadfunc, NULL);
  // Copy x into y until z is set
  while (!z) { *volatile_y = *volatile_x; }
  // Close down the worker
  pthread_join(thread, &dummy);
  // If z is 1 at this point we know that SECRET is not 1.
  return (z == 1);
}

int main() {
  unsigned count = 0;
  for(int i = 0; i < 10; i++) {
    if(attack()) count++;
  }
  printf("%u\n", count);
  return 0;
}


