#define BOOST_TEST_DYN_LINK

#include <algorithm>
#include <typeinfo>

#include <boost/test/unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/mpl/list.hpp>

#include "corpus.hpp"
#include "UnigramTagger.hpp"
#include "MarkovTagger.hpp"
#include "HMMTagger.hpp"

typedef boost::mpl::list<UnigramTagger,
			 MarkovTagger,
			 ViterbiMarkovTagger,
			 HMMTagger
			 > test_types;

BOOST_AUTO_TEST_SUITE(test)
BOOST_AUTO_TEST_CASE_TEMPLATE( Tagger_test, TaggerT, test_types )
{
  //Corpus corpus("testcorpus.tsv");
  Corpus corpus("../simplified_corpus.tsv");
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
