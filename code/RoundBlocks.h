#ifndef ROUNDBLOCKS_H
#define ROUNDBLOCKS_H

#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <mutex>

#include <wmmintrin.h>
#include <stdint.h>
#include <assert.h>
#include <random>

class RoundBlocks {
  private:
    enum byte_t {
      S0, S1, S2, S3, S4, S5, S6, S7, S8, S9, S10, S11, S12, S13, S14, S15
    };

    __m128i m_blocks[4];

    bool readURandom(uint8_t* data, uint8_t size);

  public:
    /*Block A == constant A5*/
    /*Block B == constant C3*/
    /*Block C == constant B5*/
    /*Block D == constant C5*/
    enum block_t {
      b_A , b_B, b_C, b_D
    };

    RoundBlocks();
    RoundBlocks(const RoundBlocks &obj);
    RoundBlocks(const __m128i src_a, const __m128i src_b, const __m128i src_c, const __m128i src_d);
    RoundBlocks(const __m128i src[], const uint8_t size);
    RoundBlocks(const uint8_t* src_a, const uint8_t* src_b, const uint8_t* src_c, const uint8_t* src_d);
    RoundBlocks(const uint8_t* src, const uint8_t size);

    RoundBlocks& operator=(const RoundBlocks& obj);
    RoundBlocks operator^(const RoundBlocks& obj);
    bool operator==(const RoundBlocks& obj);
    bool operator!=(const RoundBlocks& obj);
    friend std::ostream& operator<<(std::ostream& os, const RoundBlocks& obj);

    void setDiag(uint32_t i, block_t bl);
    void setConstDiff(block_t bl, uint8_t s12_diff);
    void setConstDiff(block_t bl, const uint8_t *block_diff);
    uint32_t extractDiag(block_t bl);
    void varyA5();
    void varyC5();

    __m128i getBlockA() const;
    __m128i getBlockB() const;
    __m128i getBlockC() const;
    __m128i getBlockD() const;
    void getBlocks(__m128i dest[], uint8_t size) const;
    void getBlockA(uint8_t* dest, uint8_t size) const;
    void getBlockB(uint8_t* dest, uint8_t size) const;
    void getBlockC(uint8_t* dest, uint8_t size) const;
    void getBlockD(uint8_t* dest, uint8_t size) const;
    void getBlocks(uint8_t* dest, uint8_t size) const;

    void setBlockA(const __m128i src);
    void setBlockB(const __m128i src);
    void setBlockC(const __m128i src);
    void setBlockD(const __m128i src);
    void setBlocks(const __m128i src[], const uint8_t size);
    void setBlockA(const uint8_t* src, const uint8_t size);
    void setBlockB(const uint8_t* src, const uint8_t size);
    void setBlockC(const uint8_t* src, const uint8_t size);
    void setBlockD(const uint8_t* src, const uint8_t size);
    void setBlocks(const uint8_t* src, const uint8_t size);
};


#endif
