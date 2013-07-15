
#pragma once

#include <algorithm>
#include <cctype>
#include <map>
#include <Eigen/Eigen>

#include "corpus.hpp"
#include "Tagger.hpp"

using std::make_pair;
using std::map;
using std::string;
using std::vector;

using Eigen::VectorXi;

class UnigramTagger : public Tagger {
public:
  typedef map<string, VectorXi> tagcount_t;
protected:
  tagcount_t tagcounts;
public:
  UnigramTagger() {}

  static tagcount_t::iterator newvec(tagcount_t& counts, const string& s,
				     size_t size) {
    tagcount_t::iterator wordi =
      counts.insert(make_pair(s, VectorXi(size))).first;
    for (size_t i = 0; i < wordi->second.size(); i++) {
      wordi->second[i] = 0;
    }
    return wordi;
  }

  static void updateSentenceCounts(tagcount_t& counts, const Sentence& sentence) {
    for (pair<Tag, string> taggedWord : sentence.words) {
      int tag = taggedWord.first;
      const string& word = taggedWord.second;

      // Use lower-cased words only
      string loweredWord(word.size(), '\0');

      std::transform(word.begin(), word.end(),
		     loweredWord.begin(), (int (*)(int))std::tolower);

      tagcount_t::iterator wordi = counts.find(loweredWord);
      if (wordi == counts.end()) {
	wordi = newvec(counts, loweredWord, counts[""].size());
      }
      wordi->second[tag] += 1;
      counts[""][tag] += 1;
    }
  }

  UnigramTagger(const Corpus& corpus) {
    newvec(tagcounts, "", corpus.taglib.size()); // for all words
    for (shared_ptr<Sentence> sentence : corpus.sentences)
      updateSentenceCounts(tagcounts, *sentence);
  }

  virtual void tag(vector<pair<int, string> >& words) const {
    for (pair<int, string>& word : words) {
	string loweredWord(word.second.size(), '\0');

	std::transform(word.second.begin(), word.second.end(),
		       loweredWord.begin(), (int (*)(int))std::tolower);

	tagcount_t::const_iterator i = tagcounts.find(loweredWord);
	if (i == tagcounts.end())
	  i = tagcounts.find("");
	const VectorXi& counts = i->second;
	word.first =  std::max_element(&counts[0], &counts[0]+counts.size()) - &counts[0];
    }
  }
};
