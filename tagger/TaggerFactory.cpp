
#include <map>
#include <boost/functional/factory.hpp>

#include "TaggerFactory.hpp"
#include "UnigramTagger.hpp"
#include "HMMTagger.hpp"
#include "MarkovTagger.hpp"

using std::map;

typedef boost::function<Tagger *(const Corpus& corpus)> factory_t;
static map<string, factory_t> factoryMap;

static void init_factory() {
  if (factoryMap.empty()) {
    factoryMap["UnigramTagger"] = boost::factory<UnigramTagger *>();
    factoryMap["MarkovTagger"] = boost::factory<MarkovTagger *>();
    factoryMap["ViterbiMarkovTagger"] = boost::factory<ViterbiMarkovTagger *>();
    factoryMap["HMMTagger"] = boost::factory<HMMTagger *>();
  }
}

shared_ptr<Tagger> taggerFactory(const string& className, const Corpus& corpus)
{
  init_factory();
  return shared_ptr<Tagger>(factoryMap[className](corpus));
}
