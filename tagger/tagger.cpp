
#include <fstream>
#include <iostream>

#include <boost/program_options.hpp>

#include "corpus.hpp"
#include "HMMTagger.hpp"

namespace po = boost::program_options;

double do_tagging(const Tagger& tagger,
		  const Corpus& testcorpus,
		  std::ostream& output) {
  size_t correct = 0, incorrect = 0;

  for (const shared_ptr<Sentence>& sentence : testcorpus.sentences) {
    vector<pair<int, string> > words(sentence->words);
    for (pair<int, string>& taggedWord : words)
      taggedWord.first = -1;
    tagger.tag(words);
    for (int i = 0; i < words.size(); i++) {
      output << testcorpus.taglib.tagName(words[i].first) << "\t"
	     << words[i].second << std::endl;

      if (words[i].first == sentence->words[i].first)
	correct++;
      else
	incorrect++;
    }
    output << std::endl;
  }

  return (double)incorrect / (double)(correct+incorrect);
}

int main(int argc, const char *argv[]) {
  po::options_description desc("Allowed options");
  desc.add_options()
    ("help", "produce help message")
    ("training", po::value<string>(), "training corpus filename")
    ("testing", po::value<string>(), "testing corpus filename")
    ("output", po::value<string>(), "output file")
    ;
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return 0;
  }

  std::string training_filename = "simplified_corpus.tsv";
  std::string testing_filename = training_filename;

  if (vm.count("training"))
    training_filename = vm["training"].as<std::string>();
  if (vm.count("testing"))
    testing_filename = vm["testing"].as<std::string>();

  std::ifstream is;
  is.exceptions(std::ios_base::failbit | std::ios_base::badbit);
  try {
    is.open(training_filename.c_str());
  } catch (std::exception& e) {
    std::cerr << "Error opening file " << training_filename << std::endl;
    throw;
  }
  is.exceptions(std::ios_base::goodbit);

  Corpus corpus(is);
  is.clear();
  is.close();

  std::cout << "Loaded " << corpus.size() << " sentences." << std::endl;

  HMMTagger tagger(corpus);
  std::cout << "Trained HMM." << std::endl;

  is.exceptions(std::ios_base::failbit | std::ios_base::badbit);
  try {
    is.open(testing_filename.c_str());
  } catch (std::exception& e) {
    std::cerr << "Error opening file " << testing_filename << std::endl;
    throw;
  }
  is.exceptions(std::ios_base::goodbit);
  Corpus testing(is, corpus.taglib);
  is.close();

  if (!testing.taglib.superset(corpus.taglib)) {
    std::cerr << "Incompatible corpuses!" << std::endl;
    return 1;
  }

  std::cout << "Computing error." << std::endl;
  std::ofstream output;

  if (vm.count("output")) {
    try {
      output.exceptions(std::ios_base::failbit | std::ios_base::badbit);
      output.open(vm["output"].as<std::string>().c_str());
    } catch (std::exception& e) {
      std::cerr << "Error opening file " << testing_filename << std::endl;
      throw;
    }
  }

  double error = do_tagging(tagger, testing, output);
  std::cout << "Error on testing set: " << error << std::endl;
  return 0;
}
