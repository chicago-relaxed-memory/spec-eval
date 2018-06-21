#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>  // nanosleep()
#include <stdlib.h>  // exit()
#include <math.h>
#include <string.h>
#include <x86intrin.h>  // __rdtsc()

static volatile bool alwaysFalse = false;
static unsigned x, y;

#ifndef SECRET
#error Please pass -DSECRET=n (for some value of n) when compiling.
#endif

// in the SECRET<=0 case, gcc feels the need to keep the x=1 store intact.
// in the SECRET>0 case, gcc is fine skipping the dead x=1 store and doing just the x=2 store.
// iters: a tuning parameter, see description on singleRun() below
static void* threadfunc(void* iters) {
  x = 1;
  if(alwaysFalse) {
    if(SECRET > 0) y = 1;
  } else {
    y = 1;
  }

  // waste some time, but don't use a syscall like usleep().
  // If we use a syscall, gcc wants to do the x=1 store first, regardless
  // Must be a do-while loop so gcc knows it executes at least once
  // (otherwise gcc inserts an extra conditional branch, and suddenly thinks
  // the x=1 store is necessary again)
  //uint64_t iterscount = (uint64_t)iters & 8 + 8;
  volatile int v = 0;
  //do { v++; } while(--iterscount > 0);
  //for(uint64_t i = 0; i < iterscount; i++) v++;
#define TWICE(op) op op
#define EIGHT(op) TWICE(TWICE(TWICE(op)))
#define KILO(op) TWICE(EIGHT(EIGHT(EIGHT(op))))
#define MEGA(op) KILO(KILO(op))
  EIGHT(EIGHT(__rdtsc();))

  x = 2;
  return (void*) (uintptr_t) y;  // keep gcc from removing the y=1 store entirely
}

// iters: a tuning parameter, how long other thread attempts to stall (in some arbitrary time unit) between x=1 and x=2
static int singleRun(uint64_t iters) {
  x = 0;
  y = 0;
  void* dummy;
  volatile unsigned* x_vol = &x;  // use a volatile pointer so it keeps reloading for real
                                  // but we don't want x itself to be volatile, because then DSE isn't allowed
  pthread_t thread;
  pthread_create(&thread, NULL, &threadfunc, (void*) iters);
  int a, b;
  do { a = *x_vol; } while(a == 0);  // wait until a is not 0
  b = y;
  pthread_join(thread, &dummy);
  return a;
}

// returns a timestamp in nanoseconds
static uint64_t getTime() {
  struct timespec time;
  clock_gettime(CLOCK_MONOTONIC_RAW, &time);
  return (uint64_t)time.tv_sec*1e9 + (uint64_t)time.tv_nsec;
}

struct manyRuns_res {
  unsigned numZeroes;  // how many times singleRun() returned 0
  unsigned numOnes;  // how many times singleRun() returned 1
  unsigned numTwos;  // how many times singleRun() returned 2
  unsigned numRuns;  // how many total runs were done (should be zeroes+ones+twos unless something really weird is going on)
  uint64_t elapsedNanoseconds;  // total duration of manyRuns, in nanoseconds
};

// iters: a tuning parameter see notes on singleRun()
// durationMS: how long to run the program for (approximately), in milliseconds
// checkEvery: check the time every *this many* runs, to see if we're done
// res: pointer to a manyRuns_res struct where results should be returned
void manyRuns(uint64_t iters, unsigned durationMS, unsigned checkEvery, struct manyRuns_res* res) {
  res->numZeroes = 0;
  res->numOnes = 0;
  res->numTwos = 0;
  res->numRuns = 0;
  uint64_t start = getTime();
  uint64_t targetEnd = start + durationMS*1e6;
  while(getTime() < targetEnd) {
    for(int i = 0; i < checkEvery; i++) {
      switch(singleRun(iters)) {
        case 0: res->numZeroes++; break;
        case 1: res->numOnes++; break;
        case 2: res->numTwos++; break;
        default: break;
      }
      res->numRuns++;
    }
  }
  uint64_t end = getTime();
  res->elapsedNanoseconds = end-start;
}

// These are explained in manyRuns_analyze()
struct manyRuns_analysis {
  double ns_per_run;  // how many nanoseconds each run took on average
  // the remaining three fields only make sense for SECRET=false runs
  double hitrate;  // what percentage of trials observed x=1
  double msToLeakOneBit;  // estimated time (in ms) required to leak one bit (with 99% certainty)
  double leakedBitsPerSec;  // information leak bandwidth, in bits/sec
};

// res: results to analyze
// analysis: pointer to a manyRuns_analysis struct where results should be returned
static void manyRuns_analyze(struct manyRuns_res* res, struct manyRuns_analysis* analysis) {
  analysis->ns_per_run = res->elapsedNanoseconds / (double)res->numRuns;
  double ms_per_run = analysis->ns_per_run / 1e6;
  analysis->hitrate = res->numOnes / (double)res->numRuns;
  // Suppose we need to repeat trials until the probability of a hit (given that SECRET was false) is 99%.
  // Where N is the number of trials, we desire (1-hitrate)^N < 0.01
  // N = log<base 1-hitrate> 0.01 = log 0.01 / log (1-hitrate)
  double runsNeeded = log(0.01) / log(1 - analysis->hitrate);
  analysis->msToLeakOneBit = runsNeeded * ms_per_run;
  analysis->leakedBitsPerSec = 1000*(1 / analysis->msToLeakOneBit);
}

static void printUsage(char* progname) {
  fprintf(stderr, "\n");
  fprintf(stderr, "Usage: %s tune\n", progname);
  fprintf(stderr, "    or %s run iters\n\n", progname);
  fprintf(stderr, "In the first case, print results a variety of values for the \"iters\" parameter\n");
  fprintf(stderr, "In the second case, print results for just the given value of \"iters\"\n\n");
}

int main(int argc, char* argv[]) {
  bool tuning;
  uint64_t iters;
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
    if(!sscanf(argv[2], "%llu", &iters)) {
      fprintf(stderr, "\nError reading \"iters\" argument\n");
      printUsage(argv[0]);
      exit(1);
    }
    tuning = false;
  } else {
    fprintf(stderr, "\nExpected 1 or 2 arguments, got %i\n", argc-1);
    printUsage(argv[0]);
    exit(1);
  }

  struct manyRuns_res res;

  if(tuning) {

    const uint64_t iters_vals[] = {1, 2, 5, 10, 20, 50, 100, 200, 500, 1e3, 2e3, 5e3, 10e3, 20e3, 50e3};
    printf("\n iters : leaked bits per second\n\n");

    struct manyRuns_analysis analysis;
    // in C++: for(iters : iters_vals)
    for(const uint64_t* cur_iters = &iters_vals[0];
        cur_iters < &iters_vals[1];
        cur_iters++) {
      printf("%6llu : ", *cur_iters);
      fflush(stdout);
      manyRuns(*cur_iters, 10000, 200, &res);
      manyRuns_analyze(&res, &analysis);
      printf("%.1f\n", analysis.leakedBitsPerSec);
    }

  } else {  // not tuning

    manyRuns(iters, 2000, 50, &res);
    struct manyRuns_analysis analysis;
    manyRuns_analyze(&res, &analysis);
    printf("0: %u  1: %u  2: %u\n", res.numZeroes, res.numOnes, res.numTwos);
    printf("%u runs in %.3f sec, or %.3f ms per run\n", res.numRuns, res.elapsedNanoseconds/(double)1e9, analysis.ns_per_run/1e6);
    if(!SECRET) {
      printf("  With the hitrate of %.1f%%, the time to leak one bit with 99%% accuracy is %.3f ms\n", analysis.hitrate*100, analysis.msToLeakOneBit);
      printf("  giving an information leak bandwidth of %.1f bits per second\n", analysis.leakedBitsPerSec);
    }

  }
  return 0;
}
