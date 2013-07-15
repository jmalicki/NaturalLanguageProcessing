

#include "corpus.hpp"

int main(int argc, const char *argv[]) {
  const char *filename = "simplified_corpus.tsv";

  if (argc > 1)
    filename = argv[1];

  Corpus corpus(filename);
  std::cout << "Loaded " << corpus.size() << " sentences." << std::endl;
  return 0;
}
