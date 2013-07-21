
#pragma once

#include <istream>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>

using std::set;
using std::pair;
using std::vector;
using std::istream;
using std::getline;
using std::string;

using boost::shared_ptr;
using boost::split;
using boost::is_any_of;

#include "TagLibrary.hpp"

class Sentence {
public:
  vector<pair<Tag, string> > words;
public:
  Sentence(istream& is, TagLibrary& taglib) {
    while (is.good()) {
      string line;
      getline(is, line);
      if (line.size() == 0)
	return;
    
      vector<string> parts(2);
      split(parts, line, is_any_of("\t"));
      if (parts.size() != 2)
	throw 0; // FIXME
      words.push_back(make_pair(taglib.tagNum(parts[0]),
				parts[1]));
    }
  }

  size_t size() const {
    return words.size();
  }
};

class Corpus {
public:
  set<shared_ptr<Sentence> > sentences;
  TagLibrary taglib;

public:
  Corpus(istream& is) {
    while (is.good()) {
      sentences.insert(shared_ptr<Sentence>(new Sentence(is, taglib)));
    }
  }

  size_t size() const {
    return sentences.size();
  }
};
