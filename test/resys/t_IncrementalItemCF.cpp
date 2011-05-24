///
/// @file t_IncrementalItemCF.cpp
/// @brief test IncrementalItemCF in Incremental Collaborative Filtering operations
/// @author Jun Jiang
/// @date Created 2011-04-29
///

#include <idmlib/resys/incremcf/IncrementalItemCF.h>

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/random.hpp>

#include <list>
#include <vector>
#include <algorithm> // copy
#include <iterator>
#include <string>
#include <sstream>

using namespace std;
using namespace boost;
using namespace idmlib::recommender;

namespace bfs = boost::filesystem;

namespace
{
const char* TEST_DIR_STR = "cf";

ostream_iterator<uint32_t> COUT_IT(cout, ", ");

const uint32_t MAX_ITEM_ID = 10;

struct MyItemIterator : public ItemIterator
{
    MyItemIterator(uint32_t min, uint32_t max)
        :min_(min)
        ,max_(max)
        ,now_(min)
    {
    }

    ~MyItemIterator()
    {
    }

    bool hasNext()
    {
        return now_ <= max_;
    }

    uint32_t next()
    {
        return now_++;
    }

    void reset()
    {
        now_ = min_;
    }

    const uint32_t min_;
    const uint32_t max_;
    uint32_t now_;
};

struct MyItemRescorer : public ItemRescorer
{
    MyItemRescorer(const std::list<uint32_t>& filterList)
        :filterSet_(filterList.begin(), filterList.end())
    {
    }

    ~MyItemRescorer()
    {
    }

    float rescore(uint32_t itemId, float originalScore)
    {
        return 0;
    }

    bool isFiltered(uint32_t itemId)
    {
        return filterSet_.find(itemId) != filterSet_.end();
    }

    std::set<uint32_t> filterSet_;
};

}

template <class OutputIterator>
void splitItems(const char* inputStr, OutputIterator result)
{
    uint32_t itemId;
    stringstream ss(inputStr);
    while (ss >> itemId)
    {
        *result++ = itemId;
    }
}

void checkPurchase(
    IncrementalItemCF& cfManager,
    uint32_t userId,
    const char* oldItemStr,
    const char* newItemStr
)
{
    list<uint32_t> oldItems;
    list<uint32_t> newItems;

    splitItems(oldItemStr, back_insert_iterator< list<uint32_t> >(oldItems));
    splitItems(newItemStr, back_insert_iterator< list<uint32_t> >(newItems));

    cout << "=> purchase event, userId: " << userId << ", old items: ";
    copy(oldItems.begin(), oldItems.end(), COUT_IT);
    cout << ", new items: ";
    copy(newItems.begin(), newItems.end(), COUT_IT);
    cout << endl;

    MyItemIterator itemIterator(1, MAX_ITEM_ID);

    const std::size_t totalNum = oldItems.size() + newItems.size();
    cfManager.incrementalBuild(userId, oldItems, newItems, itemIterator);
    // now newItems is moved into oldItems 
    BOOST_CHECK_EQUAL(oldItems.size(), totalNum);
    BOOST_CHECK_EQUAL(newItems.size(), 0);
}

/**
 * it get recommended items for @p userId, and compare them with @p goldItemStr.
 */
void checkUserRecommend(
    IncrementalItemCF& cfManager,
    uint32_t userId,
    const char* goldItemStr
)
{
    // sort gold items
    vector<uint32_t> goldResult;
    splitItems(goldItemStr, back_insert_iterator< vector<uint32_t> >(goldResult));
    sort(goldResult.begin(), goldResult.end());

    // get recommend items
    std::list<RecommendedItem> topItems;
    cfManager.getTopItems(goldResult.size(), userId, topItems);

    vector<uint32_t> result;
    for (std::list<RecommendedItem>::const_iterator it = topItems.begin();
        it != topItems.end(); ++it)
    {
        result.push_back(it->itemId);
    }

    cout << "\t<= recommend to user " << userId << " with items: ";
    copy(result.begin(), result.end(), COUT_IT);
    cout << endl;

    // check recommend items
    sort(result.begin(), result.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(result.begin(), result.end(),
                                  goldResult.begin(), goldResult.end());
}

/**
 * it get recommended items for @p inputItemStr, and compare them with @p goldItemStr.
 */
void checkItemRecommend(
    IncrementalItemCF& cfManager,
    const char* inputItemStr,
    const char* goldItemStr
)
{
    std::vector<uint32_t> inputItemIds;
    splitItems(inputItemStr, back_insert_iterator< vector<uint32_t> >(inputItemIds));

    // sort gold items
    vector<uint32_t> goldResult;
    splitItems(goldItemStr, back_insert_iterator< vector<uint32_t> >(goldResult));
    sort(goldResult.begin(), goldResult.end());

    // get recommend items
    std::list<RecommendedItem> topItems;
    cfManager.getTopItems(goldResult.size(), inputItemIds, topItems);

    vector<uint32_t> result;
    for (std::list<RecommendedItem>::const_iterator it = topItems.begin();
        it != topItems.end(); ++it)
    {
        result.push_back(it->itemId);
    }

    cout << "\t<= given items ";
    copy(inputItemIds.begin(), inputItemIds.end(), COUT_IT);
    cout << "recommend other items: ";
    copy(result.begin(), result.end(), COUT_IT);
    cout << endl;

    // check recommend items
    sort(result.begin(), result.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(result.begin(), result.end(),
                                  goldResult.begin(), goldResult.end());
}

struct RandomGenerators
{
    boost::mt19937 engine ;
    boost::uniform_int<> itemDistribution ;
    boost::variate_generator<mt19937, uniform_int<> > itemRandom;
    boost::uniform_int<> userDistribution;
    boost::variate_generator<mt19937, uniform_int<> > userRandom;
    boost::uniform_int<> visitDistribution;	
    boost::variate_generator<mt19937, uniform_int<> > visitRandom;

    RandomGenerators(int ITEMLIMIT, int USERLIMIT, int VISITLIMIT)
        :itemDistribution(1, ITEMLIMIT)
        ,itemRandom (engine, itemDistribution)
        ,userDistribution(1, USERLIMIT)
        ,userRandom (engine, userDistribution)
        ,visitDistribution(1, VISITLIMIT)
        ,visitRandom (engine, visitDistribution)
    {
    }

    void genItems(std::list<uint32_t>& items)
    {
        int N = visitRandom();
        for(int i = 0; i < N; ++i)
        {
            items.push_back(itemRandom());
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

    MyItemIterator itemIterator(1, 10);

    {
        IncrementalItemCF cfManager(
            cfPathStr + "/covisit", 1000,
            cfPathStr + "/sim", 1000,
            cfPathStr + "/nb.sdb", 30,
            cfPathStr + "/rec", 1000
        );

        uint32_t user1 = 1;
        checkPurchase(cfManager, user1, "", "1 2 3");

        uint32_t user2 = 2;
        checkPurchase(cfManager, user2, "", "2 3 4");
        checkUserRecommend(cfManager, user2, "1");

        checkItemRecommend(cfManager, "1", "2 3");
        checkItemRecommend(cfManager, "2", "1 3 4");
        checkItemRecommend(cfManager, "3", "1 2 4");
        checkItemRecommend(cfManager, "4", "2 3");
    }

    {
        IncrementalItemCF cfManager(
            cfPathStr + "/covisit", 1000,
            cfPathStr + "/sim.sdb", 1000,
            cfPathStr + "/nb.sdb", 30,
            cfPathStr + "/rec", 1000
        );

        // it should recommend user2 with item 1
        uint32_t user2 = 2;
        checkUserRecommend(cfManager, user2, "1");

        checkItemRecommend(cfManager, "1", "2 3");
        checkItemRecommend(cfManager, "2", "1 3 4");
        checkItemRecommend(cfManager, "3", "1 2 4");
        checkItemRecommend(cfManager, "4", "2 3");
    }
}


BOOST_AUTO_TEST_CASE(largeTest)
{
    bfs::path cfPath(TEST_DIR_STR);
    boost::filesystem::remove_all(cfPath);
    bfs::create_directories(cfPath);
    std::string cfPathStr = cfPath.string();

    int MaxITEM = 20000;
    MyItemIterator itemIterator(1, MaxITEM);

    IncrementalItemCF cfManager(
        cfPathStr + "/covisit", 10000,
        cfPathStr + "/sim.sdb", 10000,
        cfPathStr + "/nb.sdb", 30,
        cfPathStr + "/rec", 1000
    );

    int MaxUSER = 200000;
    int MaxVisitPerOrder = 6;
    RandomGenerators generators(MaxITEM, MaxUSER, MaxVisitPerOrder);

    int ORDERS = 20000;
		
    for(int i = 0; i < ORDERS; ++i)
    {
        if(i%1000 == 0)
            std::cout<<i<<" orders have been processed"<<std::endl;
        std::list<uint32_t> oldItems;
        std::list<uint32_t> newItems;
        generators.genItems(oldItems);
        generators.genItems(newItems);
        uint32_t userId = generators.genUser();
        cfManager.incrementalBuild(userId, oldItems, newItems, itemIterator);
    }

}

BOOST_AUTO_TEST_SUITE_END() 
