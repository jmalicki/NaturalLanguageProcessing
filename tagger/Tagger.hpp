
#pragma once

#include <string>
#include <utility>
#include <vector>

#include "TagLibrary.hpp"

class Tagger {
protected:
  TagLibrary taglib;
public:
  Tagger(const TagLibrary& _taglib) : taglib(_taglib) {}
  Tagger() {}

  virtual ~Tagger() {}

  virtual void tag(vector<std::pair<int, std::string> >& words) const = 0;
};

