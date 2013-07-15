
#pragma once

#include <iostream>
#include <fstream>
#include <set>
#include <vector>
#include <memory>
#include <utility>
#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/random_access_index.hpp>

using std::set;
using std::pair;
using std::vector;
using std::ifstream;
using std::getline;
using std::string;

using boost::shared_ptr;
using boost::split;
using boost::is_any_of;

typedef int Tag;

class TagLibrary {
protected:
  typedef boost::multi_index_container<
    std::string,
    boost::multi_index::indexed_by<
      boost::multi_index::hashed_unique<
	boost::multi_index::identity<std::string> >,
      boost::multi_index::random_access< >
      > > tag_container;
      
  tag_container tags;

public:
  TagLibrary() {}
  int tagNum(const std::string& s) {
    typedef tag_container::nth_index<0>::type::iterator i0;
    typedef tag_container::nth_index<1>::type::iterator i1;
    std::pair<i0, bool> name_iter =
      tags.get<0>().insert(s);
    i1 itemnumit = tags.project<1>(name_iter.first);
    return (itemnumit - tags.get<1>().begin());
  }

  const std::string& tagName(size_t n) const {
    return tags.get<1>()[n];
  }

  size_t size() const {
    return tags.size();
  }

};


class Sentence {
public:
  vector<pair<Tag, string> > words;
public:
  Sentence(ifstream& is, TagLibrary& taglib) {
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
  Corpus(const char *filename) {
    ifstream is(filename);
    while (is.good()) {
      sentences.insert(shared_ptr<Sentence>(new Sentence(is, taglib)));
    }
    is.close();
  }

  size_t size() const {
    return sentences.size();
  }
};
