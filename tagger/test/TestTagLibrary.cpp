#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>

#include "corpus.hpp"

BOOST_AUTO_TEST_SUITE(test)
BOOST_AUTO_TEST_CASE( TagLibrary_test )
{
  TagLibrary t;

  BOOST_CHECK_EQUAL( t.tagNum("foo"), 0 );
  BOOST_CHECK_EQUAL( t.tagNum("foo"), 0 );
  BOOST_CHECK_EQUAL( t.size(), 1 );
  BOOST_CHECK_EQUAL( t.tagNum("bar"), 1 );
  BOOST_CHECK_EQUAL( t.size(), 2 );
  BOOST_CHECK_EQUAL( t.tagNum("foo"), 0 );
  BOOST_CHECK_EQUAL( t.tagNum("bar"), 1 );
  BOOST_CHECK_EQUAL( t.size(), 2 );
  BOOST_CHECK_EQUAL( t.tagNum("baz"), 2 );
  BOOST_CHECK_EQUAL( t.size(), 3 );
}
BOOST_AUTO_TEST_SUITE_END()

