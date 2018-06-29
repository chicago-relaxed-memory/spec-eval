#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>

static int x = 0;
static int y = 0;

// 'volatile' ensures compiler keeps the check
static volatile bool alwaysFalse = false;

static volatile bool forwarder_exit = false;

#ifndef SECRET
#error Please define SECRET when compiling (e.g. pass -DSECRET=123 to the compiler)
#endif

// don't inline this: make gcc optimize this as one function
bool __attribute__((noinline)) attackfunc() {
  x = 1;
  // Apparently gcc will move a write past 31 reads but no more!
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
  // In the case that SECRET is 1, gcc will remove all of the if statements,
  // move the write of x below the alwaysFalse reads, and move the read
  // of y above at least the write of x
  // so the read of y is guaranteed to happen before the write
  // of x.  This means with SECRET==1, we're guaranteed to return false.
  // Otherwise the read of y won't be moved so we have a possibility of seeing
  // y = 1, and hence returning true.
  if (y) { return true; } else { return false; }
}

static void set_x_and_wait_for_y(int value) {
  // We need volatile aliases for x and y to make sure the reads and writes actually happen.
  volatile int* volatile_x = &x;
  volatile int* volatile_y = &y;

  // Set x and wait for y to agree
  *volatile_x = value;
  while(*volatile_y != value);
}

static bool attack() {
  set_x_and_wait_for_y(1);
  set_x_and_wait_for_y(0);
  return attackfunc();
}

static void* forwarder(void* dummy) {
  // We need volatile aliases for x and y to make sure the reads and writes actually happen.
  volatile int* volatile_x = &x;
  volatile int* volatile_y = &y;

  // copy x into y, until forwarder_exit is set
  while(!forwarder_exit) {
    *volatile_y = *volatile_x;
  }
}

int main() {
  // Fire up the forwarder thread
  pthread_t thread;
  pthread_create(&thread, NULL, &forwarder, NULL);

  // Count how many attacks return trune
  unsigned count = 0;
  for(int i = 0; i < 1000000; i++) {
    if(attack()) count++;
  }
  printf("%u\n", count);

  // tell the forwarder thread to exit
  forwarder_exit = true;

  return 0;
}
