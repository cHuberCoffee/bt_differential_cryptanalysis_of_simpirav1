#include "SimpiraA.h"

using sa::SimpiraA;
using std::cout;
using std::endl;
using std::ifstream;
using std::ios;

const uint8_t SimpiraA::ui_difference[] = {0xcd, 0xcd, 0x4c, 0x81,
                                           0x61, 0xa3, 0xc2, 0x61,
                                           0xa3, 0xc2, 0x61, 0x61,
                                           0x56, 0x2b, 0x2b, 0x7d};

const uint8_t SimpiraA::ui_single_diff = 0x40;

SimpiraA::SimpiraA(){
}

/**
 * calculates two rounds of simpira and check the resulting differences against
 * the input differences
 * @param     diag [description]
 * @return    if differences the same -> return true
 */
bool SimpiraA::search2RoundDiffs(uint64_t diag) {
  RoundBlocks a = RoundBlocks();
  RoundBlocks b = RoundBlocks();

  a.setDiag((uint32_t)diag, RoundBlocks::b_A);
  b.setDiag((uint32_t)diag, RoundBlocks::b_A);
  b.setBlockB(ui_difference, sizeof(ui_difference));
  b.setConstDiff(RoundBlocks::b_A, ui_single_diff);
  b.setConstDiff(RoundBlocks::b_C, ui_single_diff);

  RoundBlocks prev_diff = a ^ b;
  simpiraXRounds(std::ref(a), 2, 1);
  simpiraXRounds(std::ref(b), 2, 1);
  RoundBlocks after_diff = a ^ b;
  if (prev_diff == after_diff)
    return true;
  return false;
}

/**
 * fast search for the given diff. Uses only one simpira roundfunction call
 * and checks the resulting difference
 * @param start_range [description]
 * @param stop_range  [description]
 * @param r           [description]
 */
void SimpiraA::searchABDiffs(uint64_t start_range, uint64_t stop_range, std::vector<uint32_t>& r) {
  for (uint64_t i = start_range; i <= stop_range; i++) {
    RoundBlocks a = RoundBlocks();
    RoundBlocks b = RoundBlocks();
    RoundBlocks diff = RoundBlocks();
    diff.setBlockA(ui_difference, sizeof(ui_difference));
    a.setDiag(i, RoundBlocks::b_A);
    b.setDiag(i, RoundBlocks::b_A);
    b.setConstDiff(RoundBlocks::b_A, ui_single_diff);

    a.setBlockA(roundFunction(a.getBlockA(), _mm_setzero_si128(), 1, 4));
    b.setBlockA(roundFunction(b.getBlockA(), _mm_setzero_si128(), 1, 4));
    a = a ^ b;
    if (a == diff)
      r.push_back(i);
  }
}

/**
 * using the invers round function of Simpira to calculate input structs which
 * leads to valid diagonals in A3 and C5
 * @param valid_diagonal [description]
 */
void SimpiraA::searchInitStructA3(uint32_t valid_diagonal) {
  RoundBlocks init = RoundBlocks();
  init.setDiag(valid_diagonal, RoundBlocks::b_A);
  init.setDiag(valid_diagonal, RoundBlocks::b_C);

  simpiraInvXRounds(init, 2, 4);

  cout << "Init struct for A3: " << endl << init << endl;
  simpiraXRounds(std::ref(init), 2, 1);
  cout << "Init struct after 2 rounds: " << endl << init << endl;
}

/**
 * using the invers round function of Simpira to calculate input structs which
 * leads to valid diagonals in A5 and C7
 * @param valid_diagonal [description]
 */
void SimpiraA::searchInitStructA5(uint32_t valid_diagonal) {
  RoundBlocks round5 = RoundBlocks();
  round5.setDiag(valid_diagonal, RoundBlocks::b_A);
  round5.setDiag(valid_diagonal, RoundBlocks::b_C);

  RoundBlocks round4 = RoundBlocks();
  RoundBlocks round3 = RoundBlocks();

  round4.setBlockA(inversRoundFunction(round4.getBlockA(), _mm_setzero_si128(), 7, 4));
  round4.setBlockB(round5.getBlockA());
  round4.setBlockD(round5.getBlockC());

  round3.setDiag(valid_diagonal, RoundBlocks::b_C);
  round3.setBlockD(inversRoundFunction(round3.getBlockD(), _mm_setzero_si128(), 6, 4));
  round4.setBlockC(round3.getBlockD());
  round3.setBlockA(round4.getBlockD());
  round3.setBlockB(roundFunction(round3.getBlockA(), round4.getBlockA(), 5, 4));
  round3.setBlockC(roundFunction(round3.getBlockD(), round4.getBlockB(), 6, 4));
  simpiraInvXRounds(round3, 2, 4);
  RoundBlocks init = round3;

  cout << "Init struct for A5: " << endl << init << endl;
  simpiraXRounds(std::ref(init), 4, 1);
  cout << "Init struct after 4 rounds: " << endl << init << endl;
}


/**
 * create a table with @stop_range - @start_range elements containig the __m128i of block
 * A6_MC2 which get randomizd by varying the free bytes of A5
 * @param start_range [description]
 * @param stop_range  [description]
 * @param constants   [description]
 * @param r           [description]
 */
void SimpiraA::createTableForRound8(uint32_t start_range,
  uint32_t stop_range, RoundBlocks constants, std::unordered_map<uint32_t, __m128i>& r) {

  for (uint64_t i = start_range; i <= stop_range; i++) {
    constants.varyA5();
    RoundBlocks tmp = RoundBlocks();
    tmp.setBlockA(roundFunction(constants.getBlockA(), constants.getBlockC(), 9, 4));
    tmp.setBlockB(roundFunction(tmp.getBlockA(), _mm_setzero_si128(), 11, 4));
    r.insert({tmp.extractDiag(RoundBlocks::b_B), tmp.getBlockB()});
  }
}

/**
 * calculate @max_elements times D5_MC2 and test it against the already created
 * @param r           [description]
 * @param diags       [description]
 * @param start_range [description]
 * @param stop_range  [description]
 * @param constants   [description]
 * @param table       [description]
 */
void SimpiraA::searchInTableForRound8(std::vector<RoundBlocks>& r, std::vector<uint32_t>& diags, uint32_t start_range,
  uint32_t stop_range, RoundBlocks constants, std::unordered_map<uint32_t, __m128i>& table) {

  uint32_t const_diag = constants.extractDiag(RoundBlocks::b_B);
  for (uint64_t i = start_range; i <= stop_range; i++) {
    constants.varyC5();
    RoundBlocks tmp = RoundBlocks();
    tmp.setBlockA(roundFunction(constants.getBlockD(), constants.getBlockC(), 8, 4)); //c4
    tmp.setBlockB(roundFunction(tmp.getBlockA(), constants.getBlockB(), 6, 4));       //b4
    tmp.setBlockC(_mm_xor_si128(tmp.getBlockB(), constants.getBlockA()));             //a4_mc2
    tmp.setBlockC(inversRoundFunction(tmp.getBlockC(), _mm_setzero_si128(), 7, 4));   //d5
    tmp.setBlockD(roundFunction(tmp.getBlockC(), _mm_setzero_si128(), 10, 4));        //d5_mc2

    uint32_t created_diag = tmp.extractDiag(RoundBlocks::b_D);
    for (uint32_t valid_diag : diags) {
      uint32_t wanted_match = valid_diag ^ const_diag ^ created_diag;
      RoundBlocks round5 = RoundBlocks();
      try {
        round5.setBlockA(table.at(wanted_match));
      } catch (const std::out_of_range& oor) {
        continue;
      }
      round5.setBlockB(constants.getBlockC());
      round5.setBlockC(constants.getBlockD());
      round5.setBlockD(tmp.getBlockC());
      round5.setBlockA(inversRoundFunction(round5.getBlockA(), _mm_setzero_si128(), 11, 4));
      round5.setBlockA(inversRoundFunction(round5.getBlockA(), round5.getBlockB(), 9, 4));

      RoundBlocks init = round5;
      simpiraInvXRounds(std::ref(init), 4, 8);
      simpiraXRounds(std::ref(round5), 2, 9);

      if ((init.extractDiag(RoundBlocks::b_A) == const_diag) &&
      (round5.extractDiag(RoundBlocks::b_A) == valid_diag))
        r.push_back(init);
    }
  }
}
