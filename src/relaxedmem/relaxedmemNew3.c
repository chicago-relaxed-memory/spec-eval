#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>

static int x;
static int y;
static volatile int z;

// 'volatile' ensures compiler keeps the check
volatile bool alwaysFalse = false;

static volatile bool forwarder_exit;

#ifndef SECRET
#error Please define SECRET when compiling (e.g. pass -DSECRET=123 to the compiler)
#endif

// don't inline this: make gcc optimize this as one function
void __attribute__((noinline)) attackfunc() {
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
}

static bool attack() {
  x = 0;
  z = 0;
  volatile int* volatile_y = &y;  // volatile alias so we actually continuously reload & check
  while(*volatile_y);  // wait for forwarder to set y == 0
  attackfunc();
  // If z is 1 at this point we know that SECRET is not 1.
  return (z == 1);
}

static void* forwarder(void* dummy) {
  // We need volatile aliases for x and y to make sure the writes actually happen.
  volatile int* volatile_x = &x;
  volatile int* volatile_y = &y;

  // copy x into y, forever
  while(!forwarder_exit) {
    // do this several times per iteration so we don't check forwarder_exit all the time
    //*volatile_y = *volatile_x;
    //*volatile_y = *volatile_x;
    //*volatile_y = *volatile_x;
    //*volatile_y = *volatile_x;
    //*volatile_y = *volatile_x;
    //*volatile_y = *volatile_x;
    //*volatile_y = *volatile_x;
    *volatile_y = *volatile_x;
  }
}

int main() {
  forwarder_exit = false;
  // Fire up the forwarder thread
  pthread_t thread;
  pthread_create(&thread, NULL, &forwarder, NULL);

  unsigned count = 0;
  for(int i = 0; i < 1000000; i++) {
    if(attack()) count++;
  }

  // tell the forwarder thread to exit
  forwarder_exit = true;

  printf("%u\n", count);
  return 0;
}
