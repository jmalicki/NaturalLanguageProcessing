
#pragma once

#include <algorithm>
#include <limits>
#include <map>
#include <Eigen/Eigen>

#include "corpus.hpp"
#include "Tagger.hpp"

using std::make_pair;
using std::map;
using std::string;
using std::vector;

using Eigen::MatrixXi;
using Eigen::MatrixXd;
using Eigen::VectorXd;

class MarkovTagger : public Tagger {
protected:
  
  MatrixXi tagcounts;
  MatrixXd tagprobs;

  static void updateSentenceCounts(MatrixXi& counts, const Sentence& sentence) {
    int last_tag = -1;
    for (const pair<Tag, string>& taggedWord : sentence.words) {
      int tag = taggedWord.first;

      assert (last_tag+1 < counts.rows());
      assert(tag+1 < counts.cols());

      counts(tag+1, last_tag+1) += 1;
      last_tag = tag;
    }

    // Now update transition to final state.
    counts(counts.cols()-1, last_tag+1) += 1;
  }

  static void countsToProbs(MatrixXd& probs, const MatrixXi counts) {
    // Now form probability matrix from counts
    for (size_t col = 0; col < counts.cols(); col++) {
      double sum = 0;
      for (size_t row = 0; row < counts.rows(); row++) {
	sum += counts(row, col);
      }
      assert(sum > 0);
      for (size_t row = 0; row < counts.rows(); row++) {
	probs(row, col) = counts(row, col) / sum;
	// NaN != NaN, this tests that it's not NaN
	assert(probs(row, col) == probs(row, col));
	assert(probs(row, col) >= 0);
	assert(probs(row, col) <= 1);
      }
    }
  }

  static void initCounts(MatrixXi& counts, size_t size) {
    counts = counts.Constant(size, size,
			     1); // prior counts for Laplace smoothing, 0 for MLE
    // No transitions to initial state!
    for (size_t i = 0; i < size; i++)
      counts(0, i) = 0;
  }

public:
  MarkovTagger() {}
  MarkovTagger(const TagLibrary& taglib) : Tagger(taglib) {}

  MarkovTagger(const Corpus& corpus) : Tagger(corpus.taglib),
    tagcounts(taglib.size()+2, taglib.size()+2),
    tagprobs(taglib.size()+2, taglib.size()+2) {

    initCounts(tagcounts, taglib.size()+2);

    // Note some of the above transitions are a priori impossible
    // (i.e. around the initial and final states), but they should be
    // overwhelmed.
    tagprobs = tagprobs.Zero(corpus.taglib.size()+2,
			     corpus.taglib.size()+2);

    for (shared_ptr<Sentence> sentence : corpus.sentences)
      updateSentenceCounts(tagcounts, *sentence);

    countsToProbs(tagprobs, tagcounts);
  }

  virtual ~MarkovTagger() {}

  // This tag doesn't use Viterbi, just the most likely at each phase.
  virtual void tag(vector<pair<int, string> >& words) const {
    VectorXd state(tagprobs.cols());
    state = state.Zero(tagprobs.cols());
    state(0) = 1;

    for (pair<int, string>& word : words) {
      state = tagprobs * state;
      word.first = (std::max_element(&state[1], &state[1]+(state.size()-2)) - &state[1]);

      assert(word.first >= 0);
      assert(word.first < tagprobs.cols()-2);
    }
  }
};

class ViterbiMarkovTagger : public MarkovTagger
{
public:
  ViterbiMarkovTagger(const Corpus& corpus) : MarkovTagger(corpus) {}

  virtual ~ViterbiMarkovTagger() {}

  virtual void tag(vector<pair<int, string> >& words) const {
    MatrixXd viterbi(MatrixXd::Constant(tagprobs.cols(), words.size()+1,
					-std::numeric_limits<double>::infinity()
					));
    MatrixXi backpointer(MatrixXi::Zero(tagprobs.cols(), words.size()));

    for (size_t statenum = 1; statenum < tagprobs.cols(); statenum++) {
      viterbi(statenum, 0) = log(tagprobs(statenum, 0));
    }

    int t = 0;
    for (pair<int, string> word : words) {      
      for (size_t statenum = 1; statenum < tagprobs.cols(); statenum++) {
	double max = -std::numeric_limits<double>::infinity();
	size_t max_backpointer = 0;
	for (size_t laststate = 1; laststate < tagprobs.cols() - 1; laststate++) {
	  double thisval = viterbi(laststate, t) +
	    log(tagprobs(statenum, laststate));
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
