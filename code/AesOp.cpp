#include "AesOp.h"

__m128i aesEnc(__m128i input, __m128i key) {
  return _mm_aesenc_si128(input, key);
}

__m128i aesDec(__m128i input, __m128i key) {
  return _mm_aesdec_si128(input, key);
}

__m128i aesDecLast(__m128i input, __m128i key) {
  return _mm_aesdeclast_si128(input, key);
}

__m128i aesImc(__m128i input) {
  return _mm_aesimc_si128(input);
}

__m128i aesInvert(__m128i input, __m128i key) {
  return aesDecLast(aesImc(_mm_xor_si128(input, key)), _mm_setzero_si128());
}
