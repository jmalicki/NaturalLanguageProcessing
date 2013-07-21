#define BOOST_TEST_DYN_LINK

#include <algorithm>
#include <typeinfo>

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/mpl/list.hpp>

#include "corpus.hpp"
#include "UnigramTagger.hpp"
#include "MarkovTagger.hpp"
#include "HMMTagger.hpp"

using boost::filesystem::path;
using boost::filesystem::absolute;
using boost::filesystem::canonical;
using namespace boost::unit_test::framework;

typedef boost::mpl::list<UnigramTagger,
			 MarkovTagger,
			 ViterbiMarkovTagger,
			 HMMTagger
			 > test_types;

BOOST_AUTO_TEST_SUITE(test)
BOOST_AUTO_TEST_CASE_TEMPLATE( Tagger_test, TaggerT, test_types )
{
  //Corpus corpus("testcorpus.tsv");

  path executable_path(master_test_suite().argv[0]);
  path corpus_path(canonical(absolute(executable_path)).parent_path()
		   .parent_path() / "simplified_corpus.tsv");

  Corpus corpus(corpus_path.native().c_str());
  TaggerT tagger(corpus);

  vector<pair<int,string> > words;
  words.push_back(make_pair(-1, "The"));
  words.push_back(make_pair(-1, "quick"));
  words.push_back(make_pair(-1, "brown"));
  words.push_back(make_pair(-1, "fox"));
  words.push_back(make_pair(-1, "jumped"));
  words.push_back(make_pair(-1, "over"));
  words.push_back(make_pair(-1, "the"));
  words.push_back(make_pair(-1, "lazy"));
  words.push_back(make_pair(-1, "dog"));
  words.push_back(make_pair(-1, "."));
  tagger.tag(words);
std::cout << typeid(tagger).name() << " tagged " << words.size() << " words" << std::endl;
  for (pair<int, string>& word : words) {
    std::cout << word.second << ":" << corpus.taglib.tagName(word.first) << std::endl;
  }
}
BOOST_AUTO_TEST_SUITE_END()

