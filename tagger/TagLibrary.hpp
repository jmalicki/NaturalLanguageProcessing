#pragma once

#include <string>
#include <utility>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/random_access_index.hpp>

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
  TagLibrary(const TagLibrary& that) : tags(that.tags) {}

  virtual bool operator==(const TagLibrary& that) {
    // multi_index_container's operator== is protected,
    // so instead compare the elements along the ordered random access
    // container.
    return tags.size() == that.tags.size()
      && equal(tags.get<1>().begin(), tags.get<1>().end(),
	       that.tags.get<1>().begin());
  }

  virtual bool operator!=(const TagLibrary& that) {
    return !(*this == that);
  }

  virtual bool superset(const TagLibrary& that) {
    return tags.size() >= that.tags.size()
      && equal(that.tags.get<1>().begin(), that.tags.get<1>().end(),
	       tags.get<1>().begin());
  }

  virtual int tagNum(const std::string& s) {
    typedef tag_container::nth_index<0>::type::iterator i0;
    typedef tag_container::nth_index<1>::type::iterator i1;
    std::pair<i0, bool> name_iter =
      tags.get<0>().insert(s);
    i1 itemnumit = tags.project<1>(name_iter.first);
    return (itemnumit - tags.get<1>().begin());
  }

  virtual const std::string& tagName(size_t n) const {
    return tags.get<1>()[n];
  }

  virtual size_t size() const {
    return tags.size();
  }

};

