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

static void* attackfunc(void* dummy) {
  x = 1;
  // Apparently gcc (5.4.0) will move a read past 31 writes but no more!
  // (gcc 6.3.0 won't move the read past any of these writes, but that's ok)
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
  // and:
  //   (gcc 5.4.0) move the read of y all the way to the top of the function
  //   (gcc 6.3.0) not move the read all the way to the top, but move the
  //               write of x below the alwaysFalse reads, and move the read
  //               of y above at least the write of x
  // so in either case the read of y is guaranteed to happen before the write
  // of x.  This means with SECRET==1, we're guaranteed to get z = 2.
  // Otherwise the read of y won't be moved so we have a possibility of seeing
  // y = 1, and hence writing z = 1.
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
  pthread_create(&thread, NULL, &attackfunc, NULL);
  // Copy x into y until z is set
  while (!z) { *volatile_y = *volatile_x; }
  // Close down the worker
  pthread_join(thread, &dummy);
  // If z is 1 at this point we know that SECRET is not 1.
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
