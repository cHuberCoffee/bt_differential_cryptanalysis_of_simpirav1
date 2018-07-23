#ifndef SIMPIRAC_H
#define SIMPIRAC_H

#include <iostream>
#include <iomanip>
#include <vector>
#include <unordered_map>
#include <utility>
#include <thread>

#include <wmmintrin.h>  // for intrinsics for AES-NI
#include <stdint.h>

#include "SimpiraA.h"
#include "Simpira.h"
#include "RoundBlocks.h"

/* Byte-layout in the m128i
                          |                             |                            |     |
                   S0 |  S1 |  S2 |  S3 |  S4 |  S5 |  S6 |  S7 |  S8 |  S9 | S10 | S11 | S12 | S13 | S14 | S15
uint8_t test[] = {0x32, 0x00, 0xf6, 0xa8, 0x88, 0x5a, 0x00, 0x8d, 0x31, 0x31, 0x98, 0x00, 0x00, 0x37, 0x07, 0x34,   Block A
                  0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34,   Block B
                  0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34,   Block C
                  0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34};  Block D
*/

namespace sa {

  class SimpiraC {
    private:
      static SimpiraC* instance;
      unsigned int threads_in_use;
      SimpiraA* attacks;

      std::vector<uint32_t> diff_r2_results;
      std::unordered_map<uint32_t, __m128i> diag_map;
      std::vector<RoundBlocks> is_8_round;

      SimpiraC();

    public:

      static SimpiraC* getInstance();
      void search2RoundDiffs();
      void searchRound3_5InitStruct();
      void search8RoundInitStruct();

      /* GETTER & SETTER*/
      void setThreadsInUse(unsigned int nr_threads);
      unsigned int getThreadsInUse();
  };
}

#endif
