#ifndef SIMPIRAA_H
#define SIMPIRAA_H

#include <iostream>
#include <fstream>
#include <functional>
#include <string.h>
#include <stdint.h>
#include <unordered_map>
#include <vector>

#include <wmmintrin.h>  // for intrinsics for AES-NI

#include "AesOp.h"
#include "Simpira.h"

namespace sa {

  class SimpiraA {
    private:
      static const uint8_t ui_difference [sizeof(__m128i)];
      static const uint8_t ui_single_diff;

    public:
      SimpiraA();
      bool search2RoundDiffs(uint64_t diag);
      void static searchABDiffs(uint64_t start_range, uint64_t stop_range, std::vector<uint32_t>& r);

      void searchInitStructA3(uint32_t valid_diagonal);
      void searchInitStructA5(uint32_t valid_diagonal);

      void static createTableForRound8(uint32_t start_range,
        uint32_t stop_range, RoundBlocks constants, std::unordered_map<uint32_t, __m128i>& r);
      void static searchInTableForRound8(std::vector<RoundBlocks>& r, std::vector<uint32_t>& diags, uint32_t start_range,
        uint32_t stop_range, RoundBlocks constants, std::unordered_map<uint32_t, __m128i>& table);
  };
}

#endif
