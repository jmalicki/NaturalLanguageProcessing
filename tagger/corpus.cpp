
#include <iostream>
#include <fstream>

#include "corpus.hpp"

int main(int argc, const char *argv[]) {
  const char *filename = "simplified_corpus.tsv";

  if (argc > 1)
    filename = argv[1];

  std::ifstream is;
  is.exceptions(std::ios_base::badbit | std::ios_base::failbit);
  is.open(filename);
  is.exceptions(std::ios_base::goodbit);

  Corpus corpus(is);
  std::cout << "Loaded " << corpus.size() << " sentences." << std::endl;
  return 0;
}
