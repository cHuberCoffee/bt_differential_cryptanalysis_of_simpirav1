#ifndef SIMPIRA_H
#define SIMPIRA_H

#include <iostream>

#include <wmmintrin.h>
#include <stdint.h>

#include "AesOp.h"
#include "RoundBlocks.h"

__m128i roundFunction(__m128i i, __m128i k, uint8_t c, uint8_t b);
__m128i inversRoundFunction(__m128i i, __m128i k, uint8_t c, uint8_t b);
void simpiraXRounds(RoundBlocks& blocks, uint8_t rounds, uint8_t c);
void simpiraInvXRounds(RoundBlocks& blocks, uint8_t rounds, uint8_t c);

#endif
