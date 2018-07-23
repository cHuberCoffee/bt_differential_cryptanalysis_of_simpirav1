#ifndef AESOP_H
#define AESOP_H

#include <wmmintrin.h>  // for intrinsics for AES-NI

__m128i aesEnc(__m128i input, __m128i key);
__m128i aesDec(__m128i input, __m128i key);
__m128i aesDecLast(__m128i input, __m128i key);
__m128i aesImc(__m128i input);
__m128i aesInvert(__m128i input, __m128i key);

#endif
