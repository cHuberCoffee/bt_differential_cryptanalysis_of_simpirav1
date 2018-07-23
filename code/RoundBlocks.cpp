#include "RoundBlocks.h"

/**
  * std ctor
  */
RoundBlocks::RoundBlocks(){
  for (uint8_t i = 0; i < 4; i++) {
    m_blocks[i] = _mm_setzero_si128();
  }
}

/**
 * copy-ctor
 * @param obj [description]
 */
RoundBlocks::RoundBlocks(const RoundBlocks &obj) {
  for (uint8_t i = 0; i < 4; i++)
    m_blocks[i] = obj.m_blocks[i];
}

/**
  * ctor and load blocks
  * @param src_a [description]
  * @param src_b [description]
  * @param src_c [description]
  * @param src_d [description]
  */
RoundBlocks::RoundBlocks(const __m128i src_a, const __m128i src_b, const __m128i src_c, const __m128i src_d) {
  m_blocks[b_A] = src_a;
  m_blocks[b_B] = src_b;
  m_blocks[b_C] = src_c;
  m_blocks[b_D] = src_d;
}

/**
  * ctor and load blocks
  * @param src  [description]
  * @param size [description]
  */
RoundBlocks::RoundBlocks(const __m128i src[], const uint8_t size) {
  assert(size == (sizeof(m_blocks) / sizeof(__m128i)));
  for (uint8_t i = 0; i < size; i++)
    m_blocks[i] = src[i];
}

/**
 * ctor load __m128i from uint8_t ptr
 * @param src_a [description]
 * @param src_b [description]
 * @param src_c [description]
 * @param src_d [description]
 */
RoundBlocks::RoundBlocks(const uint8_t* src_a, const uint8_t* src_b, const uint8_t* src_c, const uint8_t* src_d) {
  m_blocks[b_A] = _mm_loadu_si128((__m128i*) src_a);
  m_blocks[b_B] = _mm_loadu_si128((__m128i*) src_b);
  m_blocks[b_C] = _mm_loadu_si128((__m128i*) src_c);
  m_blocks[b_D] = _mm_loadu_si128((__m128i*) src_d);
}

/**
 * ctor load full m_blocks from uint8_t ptr
 * @param src  [description]
 * @param size [description]
 */
RoundBlocks::RoundBlocks(const uint8_t* src, const uint8_t size) {
  assert(size == sizeof(m_blocks));
  for (uint8_t i = 0; i < 4; i++)
    m_blocks[i] = _mm_loadu_si128((__m128i*) (src + i * 16));
}

/**
 * assignment operator
 * @param obj [description]
 */
RoundBlocks& RoundBlocks::operator=(const RoundBlocks& obj) {
  if (this == &obj)
    return *this;
  for (uint8_t i = 0; i < 4; i++)
    m_blocks[i] = obj.m_blocks[i];
  return *this;
}

/**
 * xor operator overload
 * @param i [description]
 */
RoundBlocks RoundBlocks::operator^(const RoundBlocks& obj) {
  RoundBlocks tmp;
  for (uint8_t i = 0; i < 4; i++)
    tmp.m_blocks[i] = _mm_xor_si128(m_blocks[i], obj.m_blocks[i]);
  return tmp;
}

/**
 * equality operator overload
 * @param obj [description]
 */
bool RoundBlocks::operator==(const RoundBlocks& obj) {
  for (uint8_t i = 0; i < 4; i++) {
    __m128i vcmp = _mm_cmpeq_epi8(m_blocks[i], obj.m_blocks[i]);
    uint16_t vmask = _mm_movemask_epi8(vcmp);
    if (vmask != 0xffff)
      return false;
  }
  return true;
}

/**
 * unequality operator overlaod
 * @param obj [description]
 */
bool RoundBlocks::operator!=(const RoundBlocks& obj) {
  return !(*this == obj);
}

/**
 * operator<< overload print the RoundBlocks
 * @param os     [description]
 * @param blocks [description]
 */
std::ostream& operator<<(std::ostream& os, const RoundBlocks& obj) {
  alignas(sizeof(__m128i) * 4) uint8_t bl[4][16];
  _mm_store_si128((__m128i*) bl[0], obj.m_blocks[RoundBlocks::b_A]);
  _mm_store_si128((__m128i*) bl[1], obj.m_blocks[RoundBlocks::b_B]);
  _mm_store_si128((__m128i*) bl[2], obj.m_blocks[RoundBlocks::b_C]);
  _mm_store_si128((__m128i*) bl[3], obj.m_blocks[RoundBlocks::b_D]);

  os << "-----------------------------------------------------" << std::endl;
  os << "  BLOCK A   |   BLOCK B   |   BLOCK C   |   BLOCK D    " << std::endl;
  os << "-----------------------------------------------------" << std::endl;
  for (uint8_t i = 0; i < 4; i++) {
    for (uint8_t j = 0; j < 4; j++) {
      os << std::hex << std::setfill('0') << std::setw(2) << (unsigned int) bl[j][i] << " ";
      os << std::hex << std::setfill('0') << std::setw(2) << (unsigned int) bl[j][i + 4] << " ";
      os << std::hex << std::setfill('0') << std::setw(2) << (unsigned int) bl[j][i + 8] << " ";
      os << std::hex << std::setfill('0') << std::setw(2) << (unsigned int) bl[j][i + 12];
      if (j != 3)
        os << " | ";
      else
        os << std::endl;
    }
  }
  return os;
}

/**
 * set the diagonal of @bl block to @i
 * @param i  [description]
 * @param bl [description]
 */
void RoundBlocks::setDiag(uint32_t i, RoundBlocks::block_t bl) {
  uint8_t* ptr = (uint8_t*) &i;
  uint8_t tmp [sizeof(__m128i)];
  _mm_store_si128((__m128i*) tmp, m_blocks[bl]);

  *(tmp + S1) = *ptr;
  *(tmp + S6) = *(ptr + 1);
  *(tmp + S11) = *(ptr + 2);
  *(tmp + S12) = *(ptr + 3);
  m_blocks[bl] = _mm_loadu_si128((__m128i*) tmp);
}

/**
 * set the difference in block @bl of byte S12 to 0x40
 * @param bl [description]
 */
void RoundBlocks::setConstDiff(RoundBlocks::block_t bl, uint8_t s12_diff) {
  uint8_t tmp [sizeof(__m128i)];
  _mm_store_si128((__m128i*) tmp, m_blocks[bl]);
  *(tmp + S12) ^= s12_diff;
  m_blocks[bl] = _mm_loadu_si128((__m128i*) tmp);
}

/**
 * set a difference over a full block
 * @param block_diff [description]
 */
void RoundBlocks::setConstDiff(RoundBlocks::block_t bl, const uint8_t *block_diff) {
  __m128i tmp = _mm_loadu_si128((__m128i*) block_diff);
  m_blocks[bl] = _mm_xor_si128(m_blocks[bl], tmp);
}

/**
 * extract the diagonal of block @bl
 * @param  bl [description]
 * @return    [description]
 */
uint32_t RoundBlocks::extractDiag(RoundBlocks::block_t bl) {
  uint32_t diag;
  uint8_t* ptr = (uint8_t*) &diag;
  uint8_t tmp [sizeof(__m128i)];
  _mm_store_si128((__m128i*)tmp, m_blocks[bl]);

  *ptr = tmp[S1];
  *(ptr + 1) = tmp[S6];
  *(ptr + 2) = tmp[S11];
  *(ptr + 3) = tmp[S12];
  return diag;
}

/**
 * vary the free bytes of A5
 */
void RoundBlocks::varyA5() {
  uint8_t random_bytes [9];
  if (!readURandom(random_bytes, sizeof(random_bytes))) {
    return;
  }

  uint8_t bc = 0;
  uint8_t ui_data[16];
  _mm_store_si128((__m128i*) ui_data, m_blocks[b_A]);
  for (uint8_t c = 0; c < 16; c++) {
    if ((c == S0) || (c == S1) || (c == S2) || (c == S3) || (c == S6) || (c == S11) || (c == S12))
      continue;
    ui_data[c] = random_bytes[bc];
    bc++;
  }
  m_blocks[b_A] = _mm_loadu_si128((__m128i*) ui_data);
}

/**
 * vary the free bytes of C5
 */
void RoundBlocks::varyC5() {
  uint8_t random_bytes [12];
  if (!readURandom(random_bytes, sizeof(random_bytes))) {
    return;
  }

  uint8_t bc = 0;
  uint8_t ui_data[16];
  _mm_store_si128((__m128i*) ui_data, m_blocks[b_D]);
  for (uint8_t c = 0; c < 16; c++) {
    if ((c == S1) || (c == S6) || (c == S11) || (c == S12))
      continue;
    ui_data[c] = random_bytes[bc];
    bc++;
  }
  m_blocks[b_D] = _mm_loadu_si128((__m128i*) ui_data);
}

/**
 * read some random data from /dev/urandom
 * @param  data      [description]
 * @param  size      [description]
 * @return           [description]
 */
bool RoundBlocks::readURandom(uint8_t* data, uint8_t size) {
  std::random_device rd;
  uint8_t tmp[sizeof(unsigned int) * 24];

  for (uint8_t i = 0; i < 24; i++)
    *(tmp + (i * sizeof(unsigned int))) = rd();
  std::copy(tmp, (tmp + size), data);
  return true;
}

/** ---------------------------- GETTER / SETTER ----------------------------**/
__m128i RoundBlocks::getBlockA() const {
  return m_blocks[b_A];
}

__m128i RoundBlocks::getBlockB() const {
  return m_blocks[b_B];
}

__m128i RoundBlocks::getBlockC() const {
  return m_blocks[b_C];
}

__m128i RoundBlocks::getBlockD() const {
  return m_blocks[b_D];
}

void RoundBlocks::getBlocks(__m128i dest[], uint8_t size) const {
  assert(size == (sizeof(m_blocks) / sizeof(__m128i)));
  for (uint8_t i = 0; i < size; i++)
    dest[i] = m_blocks[i];
}

void RoundBlocks::getBlockA(uint8_t* dest, uint8_t size) const {
  assert(size == sizeof(__m128i));
  _mm_store_si128((__m128i*) dest, m_blocks[b_A]);
}

void RoundBlocks::getBlockB(uint8_t* dest, uint8_t size) const {
  assert(size == sizeof(__m128i));
  _mm_store_si128((__m128i*) dest, m_blocks[b_B]);
}

void RoundBlocks::getBlockC(uint8_t* dest, uint8_t size) const {
  assert(size == sizeof(__m128i));
  _mm_store_si128((__m128i*) dest, m_blocks[b_C]);
}

void RoundBlocks::getBlockD(uint8_t* dest, uint8_t size) const {
  assert(size == sizeof(__m128i));
  _mm_store_si128((__m128i*) dest, m_blocks[b_D]);
}

void RoundBlocks::getBlocks(uint8_t* dest, uint8_t size) const {
  assert(size == sizeof(m_blocks));
  for (uint8_t i = 0; i < size; i++)
    _mm_store_si128((__m128i*) (dest + i * sizeof(__m128i)), m_blocks[i]);
}

void RoundBlocks::setBlockA(const __m128i src) {
  m_blocks[b_A] = src;
}

void RoundBlocks::setBlockB(const __m128i src) {
  m_blocks[b_B] = src;
}

void RoundBlocks::setBlockC(const __m128i src) {
  m_blocks[b_C] = src;
}

void RoundBlocks::setBlockD(const __m128i src) {
  m_blocks[b_D] = src;
}

void RoundBlocks::setBlocks(const __m128i src[], const uint8_t size) {
  assert(size == (sizeof(m_blocks) / sizeof(__m128i)));
  for (uint8_t i = 0; i < size; i++)
    m_blocks[i] = src[i];
}

void RoundBlocks::setBlockA(const uint8_t* src, const uint8_t size) {
  assert(size == sizeof(__m128i));
  m_blocks[b_A] = _mm_loadu_si128((__m128i*) src);
}

void RoundBlocks::setBlockB(const uint8_t* src, const uint8_t size) {
  assert(size == sizeof(__m128i));
  m_blocks[b_B] = _mm_loadu_si128((__m128i*) src);
}

void RoundBlocks::setBlockC(const uint8_t* src, const uint8_t size) {
  assert(size == sizeof(__m128i));
  m_blocks[b_C] = _mm_loadu_si128((__m128i*) src);
}

void RoundBlocks::setBlockD(const uint8_t* src, const uint8_t size) {
  assert(size == sizeof(__m128i));
  m_blocks[b_D] = _mm_loadu_si128((__m128i*) src);
}

void RoundBlocks::setBlocks(const uint8_t* src, const uint8_t size) {
  assert(size == sizeof(m_blocks));
  for (uint8_t i = 0; i < size; i++)
    m_blocks[i] = _mm_loadu_si128((__m128i*) (src + i * sizeof(__m128i)));
}
