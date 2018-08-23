#include <stdint.h>

/*
inline void accessNCachelinesMatchingBottom12Bits(const void* const targetaddr, uint8_t* aligned_mem, unsigned N) {
  uintptr_t bottom12Bits = ((uintptr_t)targetaddr) & 0xfff;
  uintptr_t base = (uintptr_t)aligned_mem;  // has '0' in all of its bottom 12 bits
  for(unsigned i = 0; i < N; i++) {
    uint8_t* addr = (uint8_t*) (base | bottom12Bits);
    *addr = 123;
    base += 0x1000;  // advance to next page
  }
}
*/
#define accessNCachelinesMatchingBottom12Bits(targetaddr, aligned_mem, N) \
  uintptr_t bottom12Bits = ((uintptr_t)targetaddr) & 0xfff; \
  uintptr_t base = (uintptr_t)aligned_mem;  /* has '0' in all of its bottom 12 bits */ \
  for(unsigned i = 0; i < N; i++) { \
    uint8_t* restrict addr = (uint8_t*) (base | bottom12Bits); \
    *addr = 123; \
    base += 0x1000;  /* advance to next page */ \
  }

// targetaddr: address you want to evict from the L1 cache
// aligned_mem: at least 8 pages (32KB) of memory aligned to 4KB (12 bits)
/*
inline void evictFromL1(const void* const targetaddr, uint8_t* aligned_mem) {
  // To evict targetaddr from L1 cache we need to access 8 cachelines
  // that match targetaddr in bottom 12 bits.
  accessNCachelinesMatchingBottom12Bits(targetaddr, aligned_mem, 8);
}
*/
#define evictFromL1(targetaddr, aligned_mem) accessNCachelinesMatchingBottom12Bits(targetaddr, aligned_mem, 8)

// targetaddr: address you want to evict from the L1 cache
// aligned_mem: at least NUM_ADDRESSES pages of memory aligned to 4KB (12 bits)
#define NUM_ADDRESSES 96
/*
inline void evictFromL2_bruteforce(const void* const targetaddr, uint8_t* aligned_mem) {
  // To evict targetaddr from L2 cache we need to access 4 cachelines
  // that match targetaddr in bottom *physical* 16 bits, and also evict
  // it from L1.
  // We use a brute-force approach where we access NUM_ADDRESSES cachelines
  // that match targetaddr in bottom 12 bits; if NUM_ADDRESSES is 64 then
  // statistically 4 of these should match in bottom physical 16 bits if
  // we model virtual memory translation as random.
  // Values of NUM_ADDRESSES above 64 are for insurance.
  // Note this will evict targetaddr from L1 on its own, as we access
  // NUM_ADDRESSES cachelines that match in bottom 12 bits.
  accessNCachelinesMatchingBottom12Bits(targetaddr, aligned_mem, NUM_ADDRESSES);
}
*/
#define evictFromL2_bruteforce(targetaddr, aligned_mem) accessNCachelinesMatchingBottom12Bits(targetaddr, aligned_mem, NUM_ADDRESSES)
