#include "Simpira.h"

/**
 * round function of simpira
 * @param  input [description]
 * @param  key   [description]
 * @param  c     [description]
 * @param  b     [description]
 * @return       [description]
 */
__m128i roundFunction(__m128i input, __m128i key, uint8_t c, uint8_t b) {
  __m128i C = _mm_setr_epi32(c, b, 0, 0);
  return aesEnc(aesEnc(input, C), key);
}

/**
 * invert the round function of simpira
 * @param  input [description]
 * @param  key   [description]
 * @param  c     [description]
 * @param  b     [description]
 * @return       [description]
 */
__m128i inversRoundFunction(__m128i input, __m128i key, uint8_t c, uint8_t b) {
  __m128i C = _mm_setr_epi32(c, b, 0, 0);
  return aesInvert(aesInvert(input, key), C);
}

/**
 * calculate x rounds of simpira in forward direction
 * @param blocks [description]
 * @param rounds [description]
 * @param c      [description]
 */
void simpiraXRounds(RoundBlocks& blocks, uint8_t rounds, uint8_t c) {
  assert((rounds > 0) && (c > 0));

  uint8_t b = 4;
  RoundBlocks tmp = RoundBlocks();

  for (uint8_t r = 0; r < rounds; r++) {
    tmp.setBlockA(roundFunction(blocks.getBlockA(), blocks.getBlockB(), c++, b));
    tmp.setBlockB(roundFunction(blocks.getBlockD(), blocks.getBlockC(), c++, b));
    tmp.setBlockC(blocks.getBlockD());
    tmp.setBlockD(blocks.getBlockA());
    blocks = tmp;
  }
}

/**
 * invert x rounds of simpira
 * @param blocks [description]
 * @param rounds [description]
 * @param c      [description]
 */
void simpiraInvXRounds(RoundBlocks& blocks, uint8_t rounds, uint8_t c) {
  assert((rounds > 0) && (c > 0));

  uint8_t b = 4;
  RoundBlocks tmp = RoundBlocks();

  for (uint8_t r = rounds; r != 0; --r) {
    tmp.setBlockA(blocks.getBlockD());
    tmp.setBlockD(blocks.getBlockC());
    tmp.setBlockC(roundFunction(tmp.getBlockD(), blocks.getBlockB(), c--, b));
    tmp.setBlockB(roundFunction(tmp.getBlockA(), blocks.getBlockA(), c--, b));
    blocks = tmp;
  }
}
