
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
  wordtagprob_t wordTagProbs; // in log-space

public:

  double getEmitLogProb(const string& word, int state) const {
    string loweredWord(word.size(), '\0');

    std::transform(word.begin(), word.end(),
		   loweredWord.begin(), (int (*)(int))std::tolower);

    wordtagprob_t::const_iterator i = wordTagProbs.find(loweredWord);
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

    // number of pseudo-seen tags (not in training) of each type for each word
    double tag_pseudocount = 0.5;
    // number of pseudo-seen words not in training
    double word_pseudocount = 2;

    VectorXi marginalCounts = wordTagCounts[""];
    assert(marginalCounts.size() > 0);
    double total_obs = std::accumulate(&marginalCounts[0],
				       &marginalCounts[0]+marginalCounts.size(),
				       0);
    total_obs += tag_pseudocount * marginalCounts.size() *
      (wordTagCounts.size()-1+word_pseudocount);

    VectorXd marginalTagProb((marginalCounts.cast<double>().array()
			      + tag_pseudocount * (wordTagCounts.size()-1+
						   word_pseudocount)).log() -
			     log(total_obs));

    for (size_t i = 0; i < marginalTagProb.size(); i++) {
      assert(marginalTagProb(i) <= 0);
    }

    for (const pair<string, VectorXi>& wordCount : wordTagCounts) {
      double word_obs = std::accumulate(&wordCount.second[0],
					&wordCount.second[0]+
					wordCount.second.size(), 0);
      word_obs += tag_pseudocount * wordCount.second.size();
      
      // P(word) = word_obs / total_obs
      // P(tag|word) = Count(tag|word)/word_obs
      // P(tag|word)*P(word) = Count(tag|word) / total_obs
      VectorXd thisWordTagProb((wordCount.second.cast<double>().array()
				+ tag_pseudocount).log()
			       - log(total_obs));
      thisWordTagProb -= marginalTagProb;

      wordTagProbs[wordCount.first] = thisWordTagProb;

      for (size_t i = 0; i < thisWordTagProb.size(); i++) {
	assert(thisWordTagProb(i) <= 0);
      }
    }
  }

  HMMTagger() : MarkovTagger() {}
  HMMTagger(const TagLibrary& taglib) : MarkovTagger(taglib) {}

  HMMTagger(const Corpus& corpus) : MarkovTagger(corpus.taglib) {

    initCounts(tagcounts, taglib.size()+2);
    tagprobs = tagprobs.Zero(taglib.size()+2,
			     taglib.size()+2);

    UnigramTagger::newvec(wordTagCounts, "", taglib.size()); // for all words

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

  virtual ~HMMTagger() {}

  virtual void tag(vector<pair<int, string> >& words) const {
    MatrixXd viterbi(MatrixXd::Constant(tagprobs.cols()-1, words.size()+1,
					-std::numeric_limits<double>::infinity()
					));

    MatrixXi backpointer(MatrixXi::Zero(tagprobs.cols()-1, words.size()));

    for (size_t statenum = 0; statenum < tagprobs.cols()-1; statenum++) {
      viterbi(statenum, 0) = log(tagprobs(statenum+1, 0));
      assert(viterbi(statenum, 0) <= 0);
      // For ease of iteration, apply emission probabilities only in
      // update step.
    }

    int t = 0;
    for (pair<int, string> word : words) {      
      // We already looked at the initial time above, which is the
      // only time that can have the initial state, so laststate
      // will always be something else.

      // laststate goes to #cols - 2, since we ignore initial state here,
      // and final state can't be a previous state, so don't compute it.
      // We can transition *to* the final state, so we consider that in the
      // statenum loop
      for (size_t laststate = 0; laststate < tagprobs.cols() - 2; laststate++) {
	double prev_viterbi = viterbi(laststate, t);
	double logemitprob = getEmitLogProb(word.second, laststate);
	if (prev_viterbi > 0) {
	  std::cerr << "Viterbi logprob > 0 at iteration " << t
		    << "!  Value " << prev_viterbi << std::endl;
	  throw 0;
	}
	if (logemitprob > 0) {
	  std::cerr << "Emission logprob > 0 at iteration " << t
		    << "!  Value " << logemitprob << std::endl;
	  throw 0;
	}

	for (size_t statenum = 0; statenum < tagprobs.cols()-1; statenum++) {
	  double logtransprob = log(tagprobs(statenum+1, laststate+1));
	  double thisval = prev_viterbi + logemitprob + logtransprob;

	  if (logtransprob > 0) {
	    std::cerr << "Transition logprob > 0 at iteration " << t
		      << "!  Value " << logtransprob << std::endl;
	    throw 0;
	  }

	  if (thisval > viterbi(statenum,t+1)) {
	    viterbi(statenum, t+1) = thisval;
	    backpointer(statenum, t) = laststate;
	  }
	}
      }
      t++;
    }

    // Now produce backwords path
    size_t last_state = tagprobs.cols() - 2;
    while (--t >= 0) {
      words[t].first = backpointer(last_state, t);
      last_state = words[t].first;
      assert(words[t].first >= 0);
      assert(words[t].first < tagprobs.cols()-2);
    }
  }

};
