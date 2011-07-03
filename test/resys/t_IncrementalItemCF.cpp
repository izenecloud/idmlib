///
/// @file t_IncrementalItemCF.cpp
/// @brief test IncrementalItemCF in Incremental Collaborative Filtering operations
/// @author Jun Jiang
/// @date Created 2011-04-29
///

#include "ItemCFTest.h"
#include <idmlib/resys/incremcf/IncrementalItemCF.h>

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/random.hpp>

#include <util/ClockTimer.h>
#include <boost/timer.hpp>

#include <list>
#include <string>

#include <cmath>

using namespace std;
using namespace boost;
using namespace idmlib::recommender;

using namespace izenelib::util;

namespace bfs = boost::filesystem;

namespace
{
const char* TEST_DIR_STR = "cf";
}

struct RandomGenerators
{
    boost::mt19937 engine ;
    boost::uniform_int<> itemDistribution ;
    boost::variate_generator<mt19937, uniform_int<> > itemRandom;
    boost::uniform_int<> userDistribution;
    boost::variate_generator<mt19937, uniform_int<> > userRandom;
    boost::uniform_int<> newVisitDistribution;
    boost::variate_generator<mt19937, uniform_int<> > newVisitRandom;

    RandomGenerators(int ITEMLIMIT, int USERLIMIT, int NEWVISITLIMIT)
        :itemDistribution(1, ITEMLIMIT)
        ,itemRandom (engine, itemDistribution)
        ,userDistribution(1, USERLIMIT)
        ,userRandom (engine, userDistribution)
        ,newVisitDistribution(1, NEWVISITLIMIT)
        ,newVisitRandom (engine, newVisitDistribution)
    {
    }

    /**
     * to avoid assert fail in IncrementalItemCF::calcSimValue_(),
     * it only generates new items. */
    void genItems(std::list<uint32_t>& newItems)
    {
        int N = newVisitRandom();
        for(int i = 0; i < N; ++i)
        {
            newItems.push_back(itemRandom());
        }
    }

    uint32_t genUser()
    {
        return userRandom();
    }

};

BOOST_AUTO_TEST_SUITE(IncrementalItemCFTest)

BOOST_AUTO_TEST_CASE(smokeTest)
{
    bfs::path cfPath(TEST_DIR_STR);
    boost::filesystem::remove_all(cfPath);
    bfs::create_directories(cfPath);
    std::string cfPathStr = cfPath.string();

    ItemCFTest itemCFTest;
    {
        IncrementalItemCF cfManager(
            cfPathStr + "/covisit", 1024*1024,
            cfPathStr + "/sim.sdb", 1024*1024,
            cfPathStr + "/nb.sdb", 30,
            cfPathStr + "/rec", 1000
        );
        itemCFTest.setCFManager(&cfManager);

        uint32_t user1 = 1;
        itemCFTest.checkPurchase(user1, "", "1 2 3");

        uint32_t user2 = 2;
        itemCFTest.checkPurchase(user2, "", "3 4 5");

        itemCFTest.checkItemRecommend("1");
        itemCFTest.checkItemRecommend("2");
        itemCFTest.checkItemRecommend("3");
        itemCFTest.checkItemRecommend("4");
        itemCFTest.checkItemRecommend("1 2");
        itemCFTest.checkItemRecommend("1 2 3");
        itemCFTest.checkItemRecommend("1 2 3 4");
    }

    {
        IncrementalItemCF cfManager(
            cfPathStr + "/covisit", 1024*1024,
            cfPathStr + "/sim.sdb", 1024*1024,
            cfPathStr + "/nb.sdb", 30,
            cfPathStr + "/rec", 1000
        );
        itemCFTest.setCFManager(&cfManager);
        itemCFTest.checkVisitMatrix();
        itemCFTest.checkSimMatrix();

        uint32_t user1 = 1;
        itemCFTest.checkPurchase(user1, "1 2 3", "4 6 8");

        uint32_t user2 = 2;
        itemCFTest.checkPurchase(user2, "3 4 5", "6 7 9");

        uint32_t user3 = 3;
        itemCFTest.checkPurchase(user3, "", "1 3 5");
        itemCFTest.checkPurchase(user3, "1 3 5", "2 8 4");

        uint32_t user4 = 4;
        itemCFTest.checkPurchase(user4, "", "4 2 9");
        itemCFTest.checkPurchase(user4, "4 2 9", "7 6 5");

        uint32_t user5 = 5;
        itemCFTest.checkPurchase(user5, "", "1");

        itemCFTest.checkItemRecommend("1");
        itemCFTest.checkItemRecommend("2");
        itemCFTest.checkItemRecommend("3");
        itemCFTest.checkItemRecommend("4");
        itemCFTest.checkItemRecommend("1 2");
        itemCFTest.checkItemRecommend("1 2 3");
        itemCFTest.checkItemRecommend("1 2 3 4");
        itemCFTest.checkItemRecommend("1 3 5 7 9");
    }
}

BOOST_AUTO_TEST_CASE(largeTest)
{
    bfs::path cfPath(TEST_DIR_STR);
    boost::filesystem::remove_all(cfPath);
    bfs::create_directories(cfPath);
    std::string cfPathStr = cfPath.string();

    int MaxITEM = 100000;
    ItemCFTest::MyItemIterator itemIterator(1, MaxITEM);

    IncrementalItemCF cfManager(
        cfPathStr + "/covisit", 1024*1024,
        cfPathStr + "/sim.sdb", 1024*1024,
        cfPathStr + "/nb.sdb", 30,
        cfPathStr + "/rec", 1000
    );

    int MaxUSER = 500000;
    int MaxNewVisit = 10;
    RandomGenerators generators(MaxITEM, MaxUSER, MaxNewVisit);

    int ORDERS = 1000;

    ClockTimer t;

    for(int i = 0; i < ORDERS; ++i)
    {
        if(i%100 == 0)
            std::cout<<i<<" orders have been processed "<<t.elapsed()<<std::endl;
        std::list<uint32_t> oldItems;
        std::list<uint32_t> newItems;
        generators.genItems(newItems);

        uint32_t userId = generators.genUser();
        cfManager.buildMatrix(oldItems, newItems);
        cfManager.buildUserRecommendItems(userId, oldItems, itemIterator);
    }

}

BOOST_AUTO_TEST_SUITE_END() 
