
#pragma once

#include <string>
#include <utility>

class Tagger {
public:
  virtual void tag(vector<std::pair<int, std::string> >& words) const = 0;
};

