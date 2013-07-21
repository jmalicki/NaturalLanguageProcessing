
#pragma once

#include <string>
#include <boost/shared_ptr.hpp>

#include "Tagger.hpp"
#include "corpus.hpp"

using std::string;
using boost::shared_ptr;

shared_ptr<Tagger> taggerFactory(const string& className,
				 const Corpus& corpus);
