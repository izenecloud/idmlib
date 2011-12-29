///
/// @file t_ItemCoVisitation.cpp
/// @brief test ItemCoVisitation in CoVisit operations
/// @author Jun Jiang
/// @date Created 2011-04-29
///

#include "ItemCFTest.h"
#include <idmlib/resys/ItemCoVisitation.h>

#include <util/ClockTimer.h>
#include <boost/timer.hpp>

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/random.hpp>

#include <list>

using namespace std;
using namespace boost;
using namespace idmlib::recommender;
using namespace izenelib::util;

namespace bfs = boost::filesystem;

namespace
{
const char* TEST_DIR_STR = "covisit";
}

struct RandomGenerators
{
    boost::mt19937 engine ;
    boost::uniform_int<> itemDistribution ;
    boost::variate_generator<mt19937, uniform_int<> > itemRandom;
    boost::uniform_int<> userDistribution;
    boost::variate_generator<mt19937, uniform_int<> > userRandom;
    boost::uniform_int<> oldVisitDistribution;
    boost::variate_generator<mt19937, uniform_int<> > oldVisitRandom;
    boost::uniform_int<> newVisitDistribution;
    boost::variate_generator<mt19937, uniform_int<> > newVisitRandom;

    RandomGenerators(int ITEMLIMIT, int USERLIMIT, int OLDVISITLIMIT, int NEWVISITLIMIT)
        :itemDistribution(1, ITEMLIMIT)
        ,itemRandom (engine, itemDistribution)
        ,userDistribution(1, USERLIMIT)
        ,userRandom (engine, userDistribution)
        ,oldVisitDistribution(1, OLDVISITLIMIT)
        ,oldVisitRandom (engine, oldVisitDistribution)
        ,newVisitDistribution(1, NEWVISITLIMIT)
        ,newVisitRandom (engine, newVisitDistribution)
    {
    }

    void genItems(std::list<uint32_t>& oldItems, std::list<uint32_t>& newItems)
    {
        int N = oldVisitRandom();
        for(int i = 0; i < N; ++i)
        {
            oldItems.push_back(itemRandom());
        }
        N = newVisitRandom();
        for(int i = 0; i < N; ++i)
        {
            newItems.push_back(itemRandom());
        }
    }

};

BOOST_AUTO_TEST_SUITE(ItemCoVisitationTest)

BOOST_AUTO_TEST_CASE(smokeTest)
{
    bfs::path covisitPath(TEST_DIR_STR);
    boost::filesystem::remove_all(covisitPath);
    bfs::create_directories(covisitPath);

    ItemCFTest itemCFTest;
    {
        ItemCFTest::CoVisitation covisit(covisitPath.string()+"/visitdb");
        itemCFTest.setCoVisitation(&covisit);

        itemCFTest.checkVisit("", "1 2 3");
        itemCFTest.checkCoVisitResult();

        itemCFTest.checkVisit("1 2 3", "4 5 6");
        itemCFTest.checkCoVisitResult();
    }

    {
        ItemCFTest::CoVisitation covisit(covisitPath.string()+"/visitdb");
        itemCFTest.setCoVisitation(&covisit);

        itemCFTest.checkCoVisitResult();

        itemCFTest.checkVisit("", "2 4 6 8");
        itemCFTest.checkCoVisitResult();

        itemCFTest.checkVisit("2 4 6 8", "1 3 5 7");
        itemCFTest.checkCoVisitResult();
    }
}

BOOST_AUTO_TEST_CASE(largeTest)
{
    bfs::path covisitPath(TEST_DIR_STR);
    boost::filesystem::remove_all(covisitPath);
    bfs::create_directories(covisitPath);

    int MaxITEM = 20000;
    int MaxUSER = 200000;
    int MaxOldVisit = 200;
    int MaxNewVisit = 20;
    RandomGenerators generators(MaxITEM, MaxUSER, MaxOldVisit, MaxNewVisit);

    int ORDERS = 5000;

    ItemCFTest::CoVisitation covisit(covisitPath.string()+"/visitdb",10*1024*1024);

    ClockTimer t;

    for(int i = 0; i < ORDERS; ++i)
    {
        if(i%500 == 0)
        {
            std::cout << "orders: " << i
                      << ", elapsed time: " << t.elapsed() << ", "
                      << covisit.matrix() << std::endl;
        }
        std::list<uint32_t> oldItems;
        std::list<uint32_t> newItems;
        generators.genItems(oldItems,newItems);
        covisit.visit(oldItems, newItems);
    }
}

BOOST_AUTO_TEST_SUITE_END()
