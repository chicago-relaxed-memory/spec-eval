#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>  // nanosleep()
#include <stdlib.h>  // exit()
#include <math.h>
#include <string.h>
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

static volatile bool alwaysFalse = false;
static unsigned x, y;

// if the SECRET-based condition evaluates to false (i.e. the appropriate bit
//   of SECRET was 0), gcc feels the need to keep the x=bit*2+1 store intact,
//   and the main thread will probably observe x==bit*2+1.
// if the SECRET-based condition evaluates to true (i.e. the appropriate bit
//   of SECRET was 1), gcc is fine skipping the dead x=bit*2+1 store and doing
//   just the x=bit*2+2 store, and the main thread cannot observe x==bit*2+1.
// iters: a tuning parameter, how long we attempt to stall (in some arbitrary
//   time unit) between the two stores to x
#define DECLARE_THREADFUNC(bitnum) \
static void* threadfunc_##bitnum(void* iters) { \
  x = bitnum*2 + 1; \
  if(alwaysFalse) { \
    if(SECRET[bitnum/64] & (1ULL<<(bitnum & 63))) y = 1; \
  } else { \
    y = 1; \
  } \
 \
  /* waste some time, but don't use a syscall like usleep().                    */ \
  /* If we use a syscall, gcc wants to do the x=bit*2+1 store first, regardless */ \
  /* Must be a do-while loop so gcc knows it executes at least once             */ \
  /* (otherwise gcc inserts an extra conditional branch, and suddenly thinks    */ \
  /* the x=bit*2+1 store is necessary again)                                    */ \
 \
  uint64_t iterscount = (uint64_t)iters; \
  volatile int v = 0; \
  do { v++; } while(--iterscount > 0); \
 \
  x = bitnum*2 + 2; \
  return (void*) (uintptr_t) y;  /* keep gcc from removing the y=1 store entirely */ \
}

// declare all threadfuncs
FOR_0_TO_2047(DECLARE_THREADFUNC)

// array storing pointers to all 64 threadfuncs
void* (*threadfunc_array[2048])(void*);

// bitnum: which bit of the secret to leak
// iters: a tuning parameter passed on to threadfunc()
// returns the guessed value of the bit
static bool leakSingleBit(unsigned bitnum, uint64_t iters) {
  y = 0;
  volatile unsigned* x_vol = &x;  // use a volatile pointer so it keeps reloading for real
                                  // but we don't want x itself to be volatile, because then DSE isn't allowed
  pthread_t thread;
  void* (*threadfuncToUse)(void*) = threadfunc_array[bitnum];
  pthread_create(&thread, NULL, threadfuncToUse, (void*) iters);
  unsigned a;
  do { a = *x_vol; } while(a < bitnum*2+1);  // wait until we observe one of the
                                             // values written by this threadfunc
  unsigned b = y;
  void* dummy;
  pthread_join(thread, &dummy);
  return a==bitnum*2+2;  // if we observe x==bit*2+2, then the x=bit*2+1 store
                         // was (probably) eliminated; see notes on threadfunc()
}

// iters: a tuning parameter passed on to leakSingleBit()
// error_runs: how many redundant runs to do for error correction
//   a value of '1' means do no error correction
//   higher values indicate more error correction but will take longer
// returns the guessed value of the 2048-bit secret
//   as a heap-allocated array which caller is responsible for freeing
static uint64_t* leak2048bitSecret(uint64_t iters, unsigned error_runs) {
  unsigned run = 0;
  uint64_t* finalLeakedSecret = malloc(2048);  // 32 uint64_t's
  for(unsigned i = 0; i < 32; i++) finalLeakedSecret[i] = 0xffffffffffffffff;

  do {
    x = 0;
    for(unsigned which_64t = 0; which_64t < 32; which_64t++) {
      uint64_t leakedSecret = 0;  // just the 64 bits we're operating on right now
      for(unsigned bitnum = 0; bitnum < 64; bitnum++) {
        if(leakSingleBit(which_64t*64 + bitnum, iters)) leakedSecret |= 1ULL << bitnum;
      }
      // if we ever observe 0 in any position, that means we observed x=bit*2+1,
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
};

// iters: a tuning parameter passed on to leak2048bitSecret()
// error_runs: a tuning parameter passed on to leak2048bitSecret()
// durationMS: how long to run the program for (approximately), in milliseconds
// res: pointer to a manyRuns_res struct where results should be returned
void manyRuns(uint64_t iters, unsigned error_runs, uint64_t durationMS, struct manyRuns_res* res) {
  for(int i = 0; i < 65; i++) res->numOffBy[i] = 0;
  res->numRuns = 0;
  uint64_t start = getTime();
  uint64_t targetEnd = start + durationMS*1000000;
  while(getTime() < targetEnd) {
    uint64_t* guessedSecret = leak2048bitSecret(iters, error_runs);
    unsigned bitsWrong = 0;
    for(unsigned which_64t = 0; which_64t < 32; which_64t++)
      for(int i = 0; i < 64; i++) if((SECRET[which_64t] & (1ULL << i)) ^ (guessedSecret[which_64t] & (1ULL << i))) bitsWrong++;
    res->numOffBy[bitsWrong]++;
    res->numRuns++;
    free(guessedSecret);
  }
  uint64_t end = getTime();
  res->elapsedNanoseconds = end-start;
}

// These are explained in manyRuns_analyze()
struct manyRuns_analysis {
  double ns_per_run;  // how many nanoseconds each run took on average
  double completelyCorrectRate;  // what percentage of trials obtained the completely correct secret
  double leakedBitsPerSec;  // information leak bandwidth, in bits/sec
};

// res: results to analyze
// analysis: pointer to a manyRuns_analysis struct where results should be returned
static void manyRuns_analyze(struct manyRuns_res* res, struct manyRuns_analysis* analysis) {
  analysis->ns_per_run = res->elapsedNanoseconds / (double)res->numRuns;
  double sec_per_run = analysis->ns_per_run / 1e9;
  analysis->completelyCorrectRate = res->numOffBy[0] / (double)res->numRuns;
  analysis->leakedBitsPerSec = 2048 * (1 / sec_per_run);
}

static void printUsage(char* progname) {
  fprintf(stderr, "\n"
                  "Usage: %s tune\n"
                  "    or %s run iters error_runs\n\n"
                  "In the first case, print results for a variety of values for the \"iters\" and \"error_runs\" tuning parameters\n"
                  "In the second case, print results for just the given values of the tuning parameters\n\n"
                  "  \"iters\" tuning parameter: how long the signalling thread waits between its two assignments to x\n"
                  "  \"error_runs\" tuning parameter: how many redundant runs to do for error correction\n"
                  "    (a value of 1 means do no error correction)\n\n"
                  , progname, progname);
}

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

  // initialize threadfunc array
#define INITIALIZE_THREADFUNC(i) threadfunc_array[i] = threadfunc_##i;
  FOR_0_TO_2047(INITIALIZE_THREADFUNC)

  struct manyRuns_res res;

  if(tuning) {

    const uint64_t iters_vals[] = {1, 2, 5, 10, 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000, 50000};
    const unsigned error_runs_vals[] = {1, 2, 3, 4, 5, 7, 10, 20, 50};
    printf("\n rows are iters, columns are # of redundant runs\n"
           " each cell is leaked bits per second : %% of runs completely correct\n\n");

    // column headings
    printf("       ");
    // C++: for(const unsigned error_runs : error_runs_vals)
    for(const unsigned* error_runs = &error_runs_vals[0]; error_runs < &error_runs_vals[9]; error_runs++)
      printf("      %-3u       ", *error_runs);
    printf("\n\n");

    struct manyRuns_analysis analysis;
    // C++: for(const uint64_t iters : iters_vals)
    for(const uint64_t* iters = &iters_vals[0]; iters < &iters_vals[15]; iters++) {
      printf("%6llu ", *iters);  // row heading
      fflush(stdout);
      for(const unsigned* error_runs = &error_runs_vals[0]; error_runs < &error_runs_vals[9]; error_runs++) {
        manyRuns(*iters, *error_runs, 10000, &res);
        manyRuns_analyze(&res, &analysis);
        printf("%8.1f : %-5.1f", analysis.leakedBitsPerSec, 100*analysis.completelyCorrectRate);
        fflush(stdout);
      }
      printf("\n");
    }

  } else {  // not tuning

    manyRuns(iters, error_runs, 2000, &res);
    struct manyRuns_analysis analysis;
    manyRuns_analyze(&res, &analysis);
    unsigned total = 0;
    for(unsigned i = 0; i < 2049 && total != res.numRuns; i++) {
      if(i == 0 || res.numOffBy[i] > 0) printf("%u: %u\n", i, res.numOffBy[i]);
      total += res.numOffBy[i];
    }
    if(total != res.numRuns) printf("more: %u\n", res.numRuns - total);
    printf("%u runs in %.3f sec, or %.3f ms per run\n", res.numRuns, res.elapsedNanoseconds/(double)1e9, analysis.ns_per_run/1e6);
    printf("%.1f%% of runs were completely correct\n", analysis.completelyCorrectRate*100);
    printf("Information leak bandwidth approximately %.1f bits per second\n", analysis.leakedBitsPerSec);

  }
  return 0;
}
