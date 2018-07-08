#define _POSIX_C_SOURCE 199309L  // without this, clock_gettime() was complaining
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>  // memset()
#include <stdlib.h>  // malloc()
#include <time.h>
#include "for_0_to_2047.h"

// 2048-bit secret is 32 uint64_t's
static const uint64_t SECRET[32] = {
  0xf07b23a5461d8c9e, 0x198ab650cd723fe4, 0x2f567e10d4a89cb3, 0x9999999999999999,
  0xe67b23a54f1d8c90, 0x198cd650ab723ef4, 0x10567ef2d4a89cb3, 0x7777777777777777,
  0xf08b23a5461d7c9e, 0x891ab650cd723fe4, 0x2f465e10d7a89cb3, 0x9898989898989898,
  0xe76b23a5f41dc890, 0x098cd651ab723ef4, 0x10567efd24a89cb3, 0x7007007007007007,
  0xf07b23a5461c8d9e, 0x189ab650cd237fe4, 0x2f567e10d4a89cb3, 0x9999999999999999,
  0xe67b23a54f1d8c09, 0x198cd6a05b723ef4, 0x10567ef2d4a89cb3, 0x7777777777777777,
  0xf08b23a5461d7c9e, 0x891ab650cd723fe4, 0x2f465e10d78a9cb3, 0x9098909898989098,
  0xe76b23a5fcd41890, 0x098cd65ab1723ef4, 0x10567def24a89cb3, 0x7107107107107107,
};

static volatile bool f = false;
static bool canRead() {
  return f;
}

static int x = 0;
static int y = 0;

static volatile bool forwarder_exit = false;

#define DECLARE_ATTACKFUNC(bitnum) \
/* don't inline attackfuncs: make gcc optimize each one as one function */ \
bool __attribute__((noinline)) attackfunc_##bitnum() { \
  x = 1; \
  /* Apparently gcc will move a write past 31 reads but no more! */ \
  if (canRead()) { x = SECRET[bitnum/64] & (1ULL<<(bitnum & 63)) ? 1 : 0; } \
  if (canRead()) { x = SECRET[bitnum/64] & (1ULL<<(bitnum & 63)) ? 1 : 0; } \
  if (canRead()) { x = SECRET[bitnum/64] & (1ULL<<(bitnum & 63)) ? 1 : 0; } \
  if (canRead()) { x = SECRET[bitnum/64] & (1ULL<<(bitnum & 63)) ? 1 : 0; } \
  if (canRead()) { x = SECRET[bitnum/64] & (1ULL<<(bitnum & 63)) ? 1 : 0; } \
  if (canRead()) { x = SECRET[bitnum/64] & (1ULL<<(bitnum & 63)) ? 1 : 0; } \
  if (canRead()) { x = SECRET[bitnum/64] & (1ULL<<(bitnum & 63)) ? 1 : 0; } \
  if (canRead()) { x = SECRET[bitnum/64] & (1ULL<<(bitnum & 63)) ? 1 : 0; } \
  if (canRead()) { x = SECRET[bitnum/64] & (1ULL<<(bitnum & 63)) ? 1 : 0; } \
  if (canRead()) { x = SECRET[bitnum/64] & (1ULL<<(bitnum & 63)) ? 1 : 0; } \
  if (canRead()) { x = SECRET[bitnum/64] & (1ULL<<(bitnum & 63)) ? 1 : 0; } \
  if (canRead()) { x = SECRET[bitnum/64] & (1ULL<<(bitnum & 63)) ? 1 : 0; } \
  if (canRead()) { x = SECRET[bitnum/64] & (1ULL<<(bitnum & 63)) ? 1 : 0; } \
  if (canRead()) { x = SECRET[bitnum/64] & (1ULL<<(bitnum & 63)) ? 1 : 0; } \
  if (canRead()) { x = SECRET[bitnum/64] & (1ULL<<(bitnum & 63)) ? 1 : 0; } \
  if (canRead()) { x = SECRET[bitnum/64] & (1ULL<<(bitnum & 63)) ? 1 : 0; } \
  if (canRead()) { x = SECRET[bitnum/64] & (1ULL<<(bitnum & 63)) ? 1 : 0; } \
  if (canRead()) { x = SECRET[bitnum/64] & (1ULL<<(bitnum & 63)) ? 1 : 0; } \
  if (canRead()) { x = SECRET[bitnum/64] & (1ULL<<(bitnum & 63)) ? 1 : 0; } \
  if (canRead()) { x = SECRET[bitnum/64] & (1ULL<<(bitnum & 63)) ? 1 : 0; } \
  if (canRead()) { x = SECRET[bitnum/64] & (1ULL<<(bitnum & 63)) ? 1 : 0; } \
  if (canRead()) { x = SECRET[bitnum/64] & (1ULL<<(bitnum & 63)) ? 1 : 0; } \
  if (canRead()) { x = SECRET[bitnum/64] & (1ULL<<(bitnum & 63)) ? 1 : 0; } \
  if (canRead()) { x = SECRET[bitnum/64] & (1ULL<<(bitnum & 63)) ? 1 : 0; } \
  if (canRead()) { x = SECRET[bitnum/64] & (1ULL<<(bitnum & 63)) ? 1 : 0; } \
  if (canRead()) { x = SECRET[bitnum/64] & (1ULL<<(bitnum & 63)) ? 1 : 0; } \
  if (canRead()) { x = SECRET[bitnum/64] & (1ULL<<(bitnum & 63)) ? 1 : 0; } \
  if (canRead()) { x = SECRET[bitnum/64] & (1ULL<<(bitnum & 63)) ? 1 : 0; } \
  if (canRead()) { x = SECRET[bitnum/64] & (1ULL<<(bitnum & 63)) ? 1 : 0; } \
  if (canRead()) { x = SECRET[bitnum/64] & (1ULL<<(bitnum & 63)) ? 1 : 0; } \
  /* In the case that the appropriate bit of SECRET is 1, gcc will remove all  */ \
  /* of the if statements, move the write of x below the canRead() calls,      */ \
  /* and move the read of y above at least the write of x so the read of y is  */ \
  /* guaranteed to happen before the write of x.                               */ \
  /* This means with the SECRET bit being 1, we're guaranteed to return true.  */ \
  /* Otherwise the read of y won't be moved so we have a possibility of seeing */ \
  /* y = 1, and hence returning false.                                         */ \
  if (y) { return false; } else { return true; } \
}

FOR_0_TO_2047(DECLARE_ATTACKFUNC)

// array storing pointers to all 2048 attackfuncs
bool (*attackfunc_array[2048])();

static void set_x_and_wait_for_y(int value) {
  // We need volatile aliases for x and y to make sure the reads and writes actually happen.
  volatile int* volatile_x = &x;
  volatile int* volatile_y = &y;

  // Set x and wait for y to agree
  *volatile_x = value;
  while(*volatile_y != value);
}

static bool leakSingleBit(unsigned bitnum) {
  set_x_and_wait_for_y(1);
  set_x_and_wait_for_y(0);
  return attackfunc_array[bitnum]();
}

// error_runs: how many redundant runs to do for error correction
//   a value of '1' means do no error correction
// returns the guessed value of the 2048-bit secret
//   as a heap-allocated array which caller is responsible for freeing
static uint64_t* leak2048bitSecret(unsigned error_runs) {
  unsigned run = 0;
  uint64_t* finalLeakedSecret = malloc(2048);  // 32 uint64_t's
  memset(finalLeakedSecret, 0xff, 2048);
  do {
    for(unsigned which_64t = 0; which_64t < 32; which_64t++) {
      uint64_t leakedSecret = 0;  // just the 64 bits we're operating on right now
      for(unsigned bitnum = 0; bitnum < 64; bitnum++) {
        unsigned bigbitnum = which_64t*64 + bitnum;
        if(leakSingleBit(bigbitnum)) leakedSecret |= 1ULL << bitnum;
      }
      // if leakSingleBit *ever* returns false for a given position, that bit
      // *must* be 0 (because if the bit were 1, returning false is impossible)
      finalLeakedSecret[which_64t] &= leakedSecret;
    }
  } while(++run < error_runs);

  return finalLeakedSecret;
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

// returns a timestamp in nanoseconds
static uint64_t getTime() {
  struct timespec time;
  clock_gettime(CLOCK_MONOTONIC_RAW, &time);
  return (uint64_t)time.tv_sec*1000000000 + (uint64_t)time.tv_nsec;
}

struct manyTrials_res {
  unsigned numOffBy[2049];  // Each index stores how many times leak2048bitSecret()
                            // was incorrect in exactly that many bits.
                            // E.g. numOffBy[0] gives how many times it was
                            // exactly correct.
  unsigned numTrials;  // how many total trials were done (sum of entries in numOffBy)
  unsigned totalBitsCorrect;  // how many total bits were correct across all trials
  uint64_t elapsedNanoseconds;  // total duration of manyTrials, in nanoseconds
};

// error_runs: passed on to leak2048bitSecret, see notes there
// durationMS: keep doing trials for this long (approximately), in milliseconds
// res: pointer to a manyTrials_res struct where results should be returned
void manyTrials(unsigned error_runs, uint64_t durationMS, struct manyTrials_res* res) {
  // initialize res
  memset(res->numOffBy, 0, 2049*sizeof(unsigned));
  res->numTrials = 0;
  res->totalBitsCorrect = 0;

  uint64_t start = getTime();
  uint64_t targetEnd = start + (uint64_t)durationMS*1000000;
  while(getTime() < targetEnd) {
    uint64_t* guessedSecret = leak2048bitSecret(error_runs);
    unsigned bitsWrong = 0;
    for(unsigned which_64t = 0; which_64t < 32; which_64t++)
      for(int i = 0; i < 64; i++)
        if((SECRET[which_64t] & (1ULL << i)) ^ (guessedSecret[which_64t] & (1ULL << i))) bitsWrong++;

    res->totalBitsCorrect += 2048-bitsWrong;
    res->numOffBy[bitsWrong]++;
    res->numTrials++;
    free(guessedSecret);
  }
  uint64_t end = getTime();
  res->elapsedNanoseconds = end-start;
}

static void printUsage(char* progname) {
  fprintf(stderr, "\n"
                  "Usage: %s run error_runs\n"
                  "    or %s tune\n\n"
                  "In the first case, run the attack with the given value of \"error_runs\"\n"
                  "  (\"error_runs\": how many redundant runs to do for error correction;\n"
                  "  e.g. a value of 1 means do no error correction)\n\n"
                  "In the second case, run the attack with several different values of\n"
                  "  \"error_runs\", and print all results\n\n"
                  , progname, progname);
}

// how long to run trials for, in milliseconds
#define DURATION_MS 15000
int main(int argc, char* argv[]) {
  bool tuning;
  unsigned error_runs;
  if(argc == 2) {
    if(strncmp(argv[1], "tune", 5)) {
      fprintf(stderr, "\nWith only one argument, expected \"tune\" but got %s\n", argv[1]);
      printUsage(argv[0]);
      exit(1);
    }
    tuning = true;
  } else if(argc == 3) {
    if(strncmp(argv[1], "run", 4)) {
      fprintf(stderr, "\nWith two arguments, expected the first to be \"run\" but got %s\n", argv[1]);
      printUsage(argv[0]);
      exit(1);
    }
    if(!sscanf(argv[2], "%u", &error_runs)) {
      fprintf(stderr, "\nError reading \"error_runs\" argument\n");
      printUsage(argv[0]);
      exit(1);
    }
    tuning = false;
  } else {
    fprintf(stderr, "\nExpected 1 or 2 arguments, got %i\n", argc-1);
    printUsage(argv[0]);
    exit(1);
  }

  // initialize attackfunc array
#define INITIALIZE_ATTACKFUNC(i) attackfunc_array[i] = attackfunc_##i;
  FOR_0_TO_2047(INITIALIZE_ATTACKFUNC)

  // Fire up the forwarder thread
  pthread_t thread;
  pthread_create(&thread, NULL, &forwarder, NULL);

  struct manyTrials_res res;

  if(tuning) {
    const unsigned error_runs_vals[] = {1, 2, 3, 4, 5, 7, 10, 15, 20};
    const unsigned length_error_runs_vals = 9;
    printf("\n error_runs  leaked bits/sec  %% of bits correct  %% of trials correct\n");

    // C++: for(const unsigned error_runs : error_runs_vals)
    for(const unsigned* error_runs = &error_runs_vals[0];
        error_runs < &error_runs_vals[length_error_runs_vals];
        error_runs++) {
      printf(" %9u  ", *error_runs);
      fflush(stdout);

      manyTrials(*error_runs, DURATION_MS, &res);

      uint64_t leakedBits = res.numTrials*2048;
      double leakedBitsPerSec = leakedBits/(double)res.elapsedNanoseconds*(double)1e9;
      double bitAccuracy = res.totalBitsCorrect/(double)leakedBits;
      double trialsAccuracy = res.numOffBy[0]/(double)res.numTrials;
      printf("   %10.1f         %8.6f%%         %5.1f%%\n", leakedBitsPerSec, 100*bitAccuracy, 100*trialsAccuracy);
    }

  } else {  // not tuning

    manyTrials(error_runs, DURATION_MS, &res);

    printf("Number of complete 2048-bit trials which were off by x bits:\n");
    unsigned total = 0;
    for(unsigned i = 0; i < 2049 && total != res.numTrials; i++) {
      if(i == 0 || res.numOffBy[i] > 0) printf("%u: %u\n", i, res.numOffBy[i]);
      total += res.numOffBy[i];
    }

    uint64_t leakedBits = res.numTrials*2048;
    double leakedBitsPerSec = leakedBits/(double)res.elapsedNanoseconds*(double)1e9;
    double bitAccuracy = res.totalBitsCorrect/(double)leakedBits;
    double trialsAccuracy = res.numOffBy[0]/(double)res.numTrials;

    printf("%u trials in %.3f sec, or %.3f ms per trial\n", res.numTrials, res.elapsedNanoseconds/(double)1e9, res.elapsedNanoseconds/(double)res.numTrials/(double)1e6);
    printf("  (leak bandwidth %.1f bits/sec after error correction)\n", leakedBitsPerSec);
    printf("Overall, after error correction, %.3f%% of bits were correct\n", 100*bitAccuracy);
    printf("  and %.1f%% of runs were completely correct\n", 100*trialsAccuracy);

  }
  // tell the forwarder thread to exit
  forwarder_exit = true;

  return 0;
}
