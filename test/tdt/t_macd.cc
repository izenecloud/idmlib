#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <idmlib/tdt/macd_histogram.h>

BOOST_AUTO_TEST_SUITE( t_macd )

using namespace idmlib::tdt;

BOOST_AUTO_TEST_CASE(init_test )
{
    MacdHistogram<4,8,5> macd;
    macd.Print();
}

BOOST_AUTO_TEST_SUITE_END()
