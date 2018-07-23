#include <iostream>
#include <chrono>

#include "SimpiraC.h"

using std::cout;
using std::endl;
using sa::SimpiraC;

int main(int argc, char **argv) {
  cout << "[main] Start" << endl;
  SimpiraC* sc = SimpiraC::getInstance();

  if (argc < 2 || argc > 2) {
    cout << "usage: ./simpira <NUMBER OF THREADS>" << endl;
    return -1;
  }

  int th = 1;
  std::string arg = argv[1];
  try {
    size_t pos;
    th = std::stoi(arg, &pos);
    if (pos < arg.size()) {
      std::cerr << "Trailing characters after number: " << arg << '\n';
      return -1;
    }
    if (th > 150)
      throw std::out_of_range("");
  } catch (std::invalid_argument const &ex) {
    std::cerr << "Invalid number: " << arg << '\n';
    return -1;
  } catch (std::out_of_range const &ex) {
    std::cerr << "Number out of range (max 150): " << arg << '\n';
    return -1;
  }
  if (th <= 0) {
    cout << "Invalid number: " << th << endl;
    return -1;
  }

  std::chrono::seconds dur_s;
  std::chrono::minutes dur_m;

  auto start = std::chrono::high_resolution_clock::now();
  sc->setThreadsInUse(th);
  sc->search2RoundDiffs();
  sc->searchRound3_5InitStruct();
  sc->search8RoundInitStruct();
  auto end = std::chrono::high_resolution_clock::now();

  dur_m = std::chrono::duration_cast<std::chrono::minutes> (end - start);
  dur_s = std::chrono::duration_cast<std::chrono::seconds> (end - start - dur_m);
  cout << std::dec;
  cout << "[main] time: " << (unsigned int) dur_m.count() << " min " << (unsigned int) dur_s.count() << " s" << endl;

  return 0;
}
