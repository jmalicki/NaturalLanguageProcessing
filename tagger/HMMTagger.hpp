
#pragma once

#include <algorithm>
#include <limits>
#include <map>
#include <numeric>

#include <Eigen/Eigen>

#include "corpus.hpp"
#include "MarkovTagger.hpp"
#include "UnigramTagger.hpp"

using std::make_pair;
using std::map;
using std::string;
using std::vector;

using Eigen::MatrixXi;
using Eigen::MatrixXd;
using Eigen::VectorXd;

class HMMTagger : public MarkovTagger {
public:
  UnigramTagger::tagcount_t wordTagCounts;
  typedef map<string, VectorXd> wordtagprob_t;
  wordtagprob_t wordTagProbs;

public:

  double getEmitProb(const string& word, int state) const {
    wordtagprob_t::const_iterator i = wordTagProbs.find(word);
    if (i == wordTagProbs.end())
      i = wordTagProbs.find("");

    const VectorXd& probs = i->second;

    assert(state >= 0);
    assert(state < probs.size());

    return probs[state];
  }

  void invertWordProbability() {
    // We now have Prob(Tag|Word).  We want Prob(Word|Tag).
    // We can do this via Bayes' rule, a simple derivation from the definition
    // of conditional probability.  Prob(Tag|Word)=Prob(Tag & Word)/Prob(Word).
    // Prob(Word|Tag)=Prob(Tag & Word)/Prob(Tag).  Therefore,
    // Prob(Word|Tag)=Prob(Tag|Word)*Prob(Word)/Prob(Tag).

    // We have marginal tag counts in the word "".
    // We can compute marginal word probabilities by summing word counts.

    VectorXi marginalCounts = wordTagCounts[""];
    double total_obs = std::accumulate(&marginalCounts[0],
				       &marginalCounts[0]+marginalCounts.size(),
				       0);

    VectorXd marginalTagProb(marginalCounts.cast<double>() / total_obs);

    for (const pair<string, VectorXi>& wordCount : wordTagCounts) {
      double word_obs = std::accumulate(&wordCount.second[0],
					&wordCount.second[0]+
					wordCount.second.size(), 0);
      
      // P(word) = word_obs / total_obs
      // P(tag|word) = Count(tag|word)/word_obs
      // P(tag|word|*P(word) = Count(tag|word) / total_obs
      wordTagProbs[wordCount.first] = VectorXd(wordCount.second.cast<double>()
					       / total_obs);
      for (size_t i = 0; i < wordTagProbs[wordCount.first].size(); i++) {
	wordTagProbs[wordCount.first] /= marginalTagProb[i];
      }
    }
  }

  HMMTagger() : MarkovTagger() {}
  HMMTagger(const TagLibrary& taglib) : MarkovTagger(taglib) {}

  HMMTagger(const Corpus& corpus) : MarkovTagger(corpus.taglib) {

    initCounts(tagcounts, corpus.taglib.size()+2);
    tagprobs = tagprobs.Zero(corpus.taglib.size()+2,
			     corpus.taglib.size()+2);

    UnigramTagger::newvec(wordTagCounts, "", corpus.taglib.size()); // for all words

    // Apply Laplace smoothing, to account for out-of-vocabulary words.
    VectorXi& nullCounts = wordTagCounts[""];
    nullCounts = VectorXi::Constant(nullCounts.size(), 1);

    for (shared_ptr<Sentence> sentence : corpus.sentences) {
      updateSentenceCounts(tagcounts, *sentence);
      UnigramTagger::updateSentenceCounts(wordTagCounts, *sentence);
    }

    countsToProbs(tagprobs, tagcounts);
    invertWordProbability();
  }

  virtual void tag(vector<pair<int, string> >& words) const {
    MatrixXd viterbi(MatrixXd::Constant(tagprobs.cols(), words.size()+1,
					-std::numeric_limits<double>::infinity()
					));

    MatrixXi backpointer(MatrixXi::Zero(tagprobs.cols(), words.size()));

    for (size_t statenum = 1; statenum < tagprobs.cols(); statenum++) {
      viterbi(statenum, 0) = log(tagprobs(statenum, 0));
      // For ease of iteration, apply emission probabilities only in
      // update step.
    }

    int t = 0;
    for (pair<int, string> word : words) {      
      for (size_t statenum = 1; statenum < tagprobs.cols(); statenum++) {
	double max = 0;
	size_t max_backpointer = 0;
	// We already looked at the initial time above, which is the
	// only time that can have the initial state, so laststate
	// will always be something else.
	for (size_t laststate = 1; laststate < tagprobs.cols() - 1; laststate++) {
	  double thisval = -std::numeric_limits<double>::infinity();

	  // lastState-1 is because initial and final states don't emit.
	  thisval = viterbi(laststate, t)
	    + log(tagprobs(statenum, laststate))
	    + log(getEmitProb(word.second, laststate-1));

	  if (thisval > max) {
	    max = thisval;
	    max_backpointer = laststate;
	  }
	}
	backpointer(statenum, t) = max_backpointer;
	viterbi(statenum, t+1) = max;
      }
      t++;
    }

    // Now produce backwords path
    size_t last_state = tagprobs.cols() - 1;
    while (--t >= 0) {
      words[t].first = backpointer(last_state, t) - 1;
      last_state = words[t].first + 1;
      assert(words[t].first >= 0);
      assert(words[t].first < tagprobs.cols()-1);
    }
  }

};
