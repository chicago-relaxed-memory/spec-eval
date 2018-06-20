#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>  // nanosleep()
#include <stdlib.h>  // exit()
#include <math.h>
#include <string.h>

static volatile bool alwaysFalse = false;
static unsigned x, y;

#ifndef SECRET
#error Please pass either -DSECRET=true or -DSECRET=false when compiling.
#endif

// in the SECRET=false case, gcc feels the need to keep the x=1 store intact.
// in the SECRET=true case, gcc is fine skipping the dead x=1 store and doing just the x=2 store.
// iters: a tuning parameter, see description on singleRun() below
static void* threadfunc(void* iters) {
  x = 1;
  if(alwaysFalse) {
    if(SECRET) y = 1;
  } else {
    y = 1;
  }

  // waste some time, but don't use a syscall like usleep().
  // If we use a syscall, gcc wants to do the x=1 store first, regardless
  volatile int v = 0;
  for(unsigned i = 0; i < (uint64_t)iters; i++) v++;

  x = 2;
  return (void*) (uintptr_t) y;  // keep gcc from removing the y=1 store entirely
}

// sleepNS: a tuning parameter, how long main thread waits (in nanoseconds) before reading x
// iters: a tuning parameter, how long other thread attempts to stall (in some arbitrary time unit) between x=1 and x=2
static int singleRun(unsigned sleepNS, uint64_t iters) {
  x = 0;
  y = 0;
  int a, b;
  void* dummy;
  pthread_t thread;
  pthread_create(&thread, NULL, &threadfunc, (void*) iters);
  struct timespec sleeptime;
  sleeptime.tv_sec = sleepNS / 1000000000;
  sleeptime.tv_nsec = sleepNS % 1000000000;
  nanosleep(&sleeptime, NULL);
  a = x;
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

// sleepNS: a tuning parameter, see notes on singleRun()
// iters: a tuning parameter see notes on singleRun()
// durationMS: how long to run the program for (approximately), in milliseconds
// checkEvery: check the time every *this many* runs, to see if we're done
// res: pointer to a manyRuns_res struct where results should be returned
void manyRuns(unsigned sleepNS, uint64_t iters, unsigned durationMS, unsigned checkEvery, struct manyRuns_res* res) {
  res->numZeroes = 0;
  res->numOnes = 0;
  res->numTwos = 0;
  res->numRuns = 0;
  uint64_t start = getTime();
  uint64_t targetEnd = start + durationMS*1e6;
  while(getTime() < targetEnd) {
    for(int i = 0; i < checkEvery; i++) {
      switch(singleRun(sleepNS, iters)) {
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
  fprintf(stderr, "    or %s sleepNS iters\n\n", progname);
  fprintf(stderr, "In the first case, print results for exploring the parameter space of the tuning parameters \"sleepNS\" and \"iters\"\n");
  fprintf(stderr, "In the second case, print results for just the given parameters\n\n");
}

int main(int argc, char* argv[]) {
  bool tuning;
  unsigned sleepNS;
  uint64_t iters;
  if(argc == 2) {
    char tune[8];
    if(!sscanf(argv[1], "%7s", tune) || strncmp(tune, "tune", 7)) {
      fprintf(stderr, "\nWith only one argument, expected \"tune\" but got %s\n", argv[1]);
      printUsage(argv[0]);
      exit(1);
    }
    tuning = true;
  } else if(argc == 3) {
    if(!sscanf(argv[1], "%u", &sleepNS)) {
      fprintf(stderr, "\nError reading \"sleepNS\" argument\n");
      printUsage(argv[0]);
      exit(1);
    }
    if(!sscanf(argv[2], "%u", &iters)) {
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

    const unsigned sleepNS_vals[] = {1, 10, 20, 50, 100, 200, 500, 1000, 2000};
    const uint64_t iters_vals[] = {20e3, 50e3, 100e3, 200e3, 500e3, 1e6, 2e6};
    printf("\nRows are sleepNS, columns are iters\n");
    printf("\n    ");

    // in C++: for(iters : iters_vals)
    for(const uint64_t* cur_iters = &iters_vals[0];
        cur_iters < &iters_vals[7];
        cur_iters++) {
      printf(" %8lu", *cur_iters);
    }
    printf("\n\n");

    struct manyRuns_analysis analysis;
    // in C++: for(sleepNS : sleepNS_vals)
    for(const unsigned* cur_sleepNS = &sleepNS_vals[0];
        cur_sleepNS < &sleepNS_vals[9];
        cur_sleepNS++) {
      printf("%4u", *cur_sleepNS);
      fflush(stdout);
      // in C++: for(iters : iters_vals)
      for(const uint64_t* cur_iters = &iters_vals[0];
          cur_iters < &iters_vals[7];
          cur_iters++) {
        manyRuns(*cur_sleepNS, *cur_iters, 20000, 500, &res);
        manyRuns_analyze(&res, &analysis);
        printf(" %8.1f", analysis.leakedBitsPerSec);
        fflush(stdout);
      }
      printf("\n");
    }

  } else {  // not tuning

    manyRuns(sleepNS, iters, 2000, 50, &res);
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
