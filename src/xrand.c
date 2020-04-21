#include "xrand.h"
#include "sized_types.h"

uint64_t XRAND_SEED;

uint64_t xrand(void) {
  XRAND_SEED ^= XRAND_SEED << 1;
  XRAND_SEED ^= XRAND_SEED >> 13;
  XRAND_SEED ^= XRAND_SEED << 45;
  return XRAND_SEED;
}
