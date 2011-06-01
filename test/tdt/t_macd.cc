#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <idmlib/tdt/macd_histogram.h>
#include <idmlib/tdt/ema.h>

BOOST_AUTO_TEST_SUITE( t_macd )

using namespace idmlib::tdt;

BOOST_AUTO_TEST_CASE(init_test )
{
    std::cout<<"print macd"<<std::endl;
    MacdHistogram<4,8,5> macd;
    macd.Print();
    std::cout<<"print macd finished"<<std::endl;
}

BOOST_AUTO_TEST_CASE(ema_print )
{
    std::cout<<"print ema"<<std::endl;
    Ema<5> ema;
    ema.Print();
    std::cout<<"print ema finished"<<std::endl;
}

BOOST_AUTO_TEST_SUITE_END()
