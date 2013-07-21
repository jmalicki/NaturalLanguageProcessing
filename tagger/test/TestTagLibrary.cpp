#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>

#include "TagLibrary.hpp"

BOOST_AUTO_TEST_SUITE( TagLibrary_test )
BOOST_AUTO_TEST_CASE( basic )
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
