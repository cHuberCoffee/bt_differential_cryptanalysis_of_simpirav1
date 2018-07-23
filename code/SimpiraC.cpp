#include "SimpiraC.h"

using sa::SimpiraC;
using std::cout;
using std::endl;
using std::setfill;
using std::setw;
using std::hex;
using std::dec;

SimpiraC* SimpiraC::instance = 0;

/**
 * ctor
 */
SimpiraC::SimpiraC() : threads_in_use(1){
  attacks = new SimpiraA();
}

/**
 * search 2^32 values as the diagonal (S1, S6, S11, S12) with the given differences
 * and double check it with simpira 2 rounds. The successful values are saved in
 * vector of uint32_t
 */
void SimpiraC::search2RoundDiffs() {
  cout << "[search2RoundDiffs] Search for the diffs" << endl;

  uint32_t ranges = 0;
  uint32_t to_search = UINT32_MAX;
  uint32_t range = to_search / threads_in_use;
  uint32_t rest = to_search - (threads_in_use * range);

  std::vector<std::thread> search_threads;
  std::vector<uint32_t> results [threads_in_use];

  for (unsigned int i = 0; i < threads_in_use; i++) {
    uint32_t start_range = ranges;
    if (i == (threads_in_use - 1))
      ranges = ranges + range + rest + 1;
    else
      ranges += range;
    uint32_t stop_range = (ranges - 1);
    search_threads.push_back(std::thread(&(attacks->searchABDiffs), start_range, stop_range, std::ref(results[i])));
  }

  for (auto &t : search_threads)
    t.join();

  for (std::vector<uint32_t> r : results) {
    diff_r2_results.insert(diff_r2_results.end(), r.begin(), r.end());
  }

  cout << "[search2RoundDiffs] Found nr. of valuse: " << diff_r2_results.size() << endl;
  for (auto &val : diff_r2_results)
    cout << "[search2RoundDiffs] Value: " << dec << val << " (" << setfill('0')
      << setw(8) << hex << val << ") as diagonal" << endl;

  for (auto it = diff_r2_results.begin(); it!= diff_r2_results.end(); it++) {
    if (!attacks->search2RoundDiffs(*it)) {
      cout << "[search2RoundDiffs] Delete " << *it << " from vector" << endl;
      diff_r2_results.erase(it);
    }
  }
}

/**
 * the blocks C5, A3, C3, A1 must contain valid diagonals.
 * A1 = C3
 * A3 = C5
 * this means by setting a valid diagonal of A3 & C3 and computing back
 * to the start of simpira we get a valid inital struct
 */
void SimpiraC::searchRound3_5InitStruct() {
  uint32_t valid_diagonal = 3379824225; // for now hardcorded!
  // uint32_t valid_diagonal = this->diff_r2_results[15];

  cout << "[searchInitStruct] Use " << valid_diagonal << " (" << setfill('0')
    << setw(8) << hex << valid_diagonal << ") as diagonal" << endl;

  attacks->searchInitStructA3(valid_diagonal);
  attacks->searchInitStructA5(valid_diagonal);

  cout << "[searchInitStruct] Done" << endl;
}

/**
 * searching a init-struct such that A7 & C9 contain a valid diagonal
 * A7 = C 9
 * to achieve content of A5 and C5 must be varied and D5_MC2 and A6_MC2
 * must match in the diagonal. This results a cancelation of the diagonal
 * and the diagonal of C5 is passed directly to A7
 */
void SimpiraC::search8RoundInitStruct() {
  uint32_t to_search = (UINT16_MAX);
  // uint32_t to_search = (UINT16_MAX * 256); /*2^24*/
  uint32_t ranges = 0;
  uint32_t range = to_search / threads_in_use;
  uint32_t rest = to_search - (threads_in_use * range);


  for (uint32_t valid_diagonal : diff_r2_results) {
    diag_map.clear();
    std::vector<std::thread> creater_threads;
    std::unordered_map<uint32_t, __m128i> results [threads_in_use];
    std::vector<RoundBlocks> vec_res [threads_in_use];

    cout << "[search8RoundInitStruct] Use " << dec << valid_diagonal << " (" << setfill('0')
    << setw(8) << hex << valid_diagonal << ") as diagonal" << endl;

    RoundBlocks constants = RoundBlocks();
    constants.setDiag(valid_diagonal, RoundBlocks::b_B);
    constants.setDiag(valid_diagonal, RoundBlocks::b_D);
    constants.setBlockA(constants.getBlockB());

    for (unsigned int i = 0; i < threads_in_use; i++) {
      uint32_t start_range = ranges;
      if (i == (threads_in_use - 1))
        ranges = ranges + range + rest + 1;
      else
        ranges += range;
      uint32_t stop_range = (ranges - 1);
      creater_threads.push_back(std::thread(&(attacks->createTableForRound8),
        start_range, stop_range, constants, std::ref(results[i])));
    }

    for (auto &t : creater_threads)
      t.join();
    creater_threads.clear();
    for (std::unordered_map<uint32_t, __m128i> r : results)
      diag_map.insert(r.begin(), r.end());

    /* ----------------- SEARCH PART ----------------- */
    ranges = 0;
    for (unsigned int i = 0; i < threads_in_use; i++) {
      uint32_t start_range = ranges;
      if (i == (threads_in_use - 1))
        ranges = ranges + range + rest + 1;
      else
        ranges += range;
      uint32_t stop_range = (ranges - 1);
      creater_threads.push_back(std::thread(&(attacks->searchInTableForRound8), std::ref(vec_res[i]), std::ref(diff_r2_results),
        start_range, stop_range, constants, std::ref(diag_map)));
    }

    for (auto &t : creater_threads)
      t.join();
    for (std::vector<RoundBlocks> r : vec_res)
      is_8_round.insert(is_8_round.end(), r.begin(), r.end());
  }

  cout << "[search8RoundInitStruct] Found Structs: " << std::dec << is_8_round.size() << endl;
  cout << "[search8RoundInitStruct] Done" << endl;
}



/* GETTER & SETTER -----------------------------------------------------------*/
void SimpiraC::setThreadsInUse(unsigned int nr_threads) {
  if (nr_threads > 150)
    this->threads_in_use = 150;
  else
    this->threads_in_use = nr_threads;
  cout << "[SimpiraC] set " << threads_in_use << " threads" << endl;
}

unsigned int SimpiraC::getThreadsInUse() {
  return this->threads_in_use;
}

SimpiraC* SimpiraC::getInstance() {
  if (!instance)
    instance = new SimpiraC();
  return instance;
}
