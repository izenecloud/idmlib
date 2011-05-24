///
/// @file t_ItemCoVisitation.cpp
/// @brief test ItemCoVisitation in CoVisit operations
/// @author Jun Jiang
/// @date Created 2011-04-29
///

#include <idmlib/resys/ItemCoVisitation.h>

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/random.hpp>

#include <list>
#include <vector>
#include <algorithm> // copy
#include <iterator>

using namespace std;
using namespace boost;
using namespace idmlib::recommender;

namespace bfs = boost::filesystem;

namespace
{
const char* TEST_DIR_STR = "covisit";

ostream_iterator<ItemType> COUT_IT(cout, ", ");
}

typedef ItemCoVisitation<CoVisitFreq> CoVisitManager;

template <class OutputIterator>
void splitItems(const char* inputStr, OutputIterator result)
{
    ItemType itemId;
    stringstream ss(inputStr);
    while (ss >> itemId)
    {
        *result++ = itemId;
    }
}

void checkVisit(
    CoVisitManager& coVisitManager,
    uint32_t userId,
    const char* oldItemStr,
    const char* newItemStr
)
{
    list<uint32_t> oldItems;
    list<uint32_t> newItems;

    splitItems(oldItemStr, back_insert_iterator< list<uint32_t> >(oldItems));
    splitItems(newItemStr, back_insert_iterator< list<uint32_t> >(newItems));

    cout << "=> visit event, userId: " << userId << ", old items: ";
    copy(oldItems.begin(), oldItems.end(), COUT_IT);
    cout << ", new items: ";
    copy(newItems.begin(), newItems.end(), COUT_IT);
    cout << endl;

    const std::size_t totalNum = oldItems.size() + newItems.size();
    coVisitManager.visit(oldItems, newItems);
    // now newItems is moved into oldItems 
    BOOST_CHECK_EQUAL(oldItems.size(), totalNum);
    BOOST_CHECK_EQUAL(newItems.size(), 0);
}

/**
 * it get covisit items for the 1st item in @p inputItemStr,
 * and compare them with @p goldItemStr.
 */
void checkCoVisitResult(
    CoVisitManager& coVisitManager,
    const char* inputItemStr,
    const char* goldItemStr
)
{
    std::vector<ItemType> inputItemIds;
    splitItems(inputItemStr, back_insert_iterator< vector<ItemType> >(inputItemIds));
    ItemType inputItem = 0;
    if (! inputItemIds.empty())
    {
        inputItem = inputItemIds.front();
    }

    // sort gold items
    vector<ItemType> goldResult;
    splitItems(goldItemStr, back_insert_iterator< vector<ItemType> >(goldResult));
    sort(goldResult.begin(), goldResult.end());

    // get covisit items
    vector<ItemType> result;
    coVisitManager.getCoVisitation(goldResult.size(), inputItem, result);

    cout << "\t<= given item " << inputItem << ", recommend covisit items: ";
    copy(result.begin(), result.end(), COUT_IT);
    cout << endl;

    // check covisit items
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

};

BOOST_AUTO_TEST_SUITE(ItemCoVisitationTest)

BOOST_AUTO_TEST_CASE(smokeTest)
{
    bfs::path covisitPath(TEST_DIR_STR);
    boost::filesystem::remove_all(covisitPath);
    bfs::create_directories(covisitPath);

    {
        CoVisitManager coVisitManager(covisitPath.string()+"/visitdb");

        uint32_t user1 = 1;
        checkVisit(coVisitManager, user1, "", "1 2 3");
        checkCoVisitResult(coVisitManager, "1", "2 3");
        checkCoVisitResult(coVisitManager, "2", "1 3");
        checkCoVisitResult(coVisitManager, "3", "1 2");
    }

    {
        CoVisitManager coVisitManager(covisitPath.string()+"/visitdb");

        checkCoVisitResult(coVisitManager, "1", "2 3");
        checkCoVisitResult(coVisitManager, "2", "1 3");
        checkCoVisitResult(coVisitManager, "3", "1 2");

        uint32_t user2 = 2;
        checkVisit(coVisitManager, user2, "", "4 5 6");
        checkCoVisitResult(coVisitManager, "4", "5 6");
        checkCoVisitResult(coVisitManager, "5", "4 6");
        checkCoVisitResult(coVisitManager, "6", "4 5");

        uint32_t user3 = 3;
        checkVisit(coVisitManager, user3, "", "2 5 7 8 9");
        checkCoVisitResult(coVisitManager, "2", "1 3 5 7 8 9");
        checkCoVisitResult(coVisitManager, "5", "2 4 6 7 8 9");
        checkCoVisitResult(coVisitManager, "7", "2 5 8 9");
        checkCoVisitResult(coVisitManager, "8", "2 5 7 9");
        checkCoVisitResult(coVisitManager, "9", "2 5 7 8");
    }
}

BOOST_AUTO_TEST_CASE(largeTest)
{
    bfs::path covisitPath(TEST_DIR_STR);
    boost::filesystem::remove_all(covisitPath);
    bfs::create_directories(covisitPath);

    int MaxITEM = 20000;
    int MaxUSER = 200000;
    int MaxVisitPerOrder = 6;
    RandomGenerators generators(MaxITEM, MaxUSER, MaxVisitPerOrder);

    int ORDERS = 200000;

    CoVisitManager coVisitManager(covisitPath.string()+"/visitdb");

    for(int i = 0; i < ORDERS; ++i)
    {
        std::list<uint32_t> oldItems;
        std::list<uint32_t> newItems;
        generators.genItems(oldItems);
        generators.genItems(newItems);
        coVisitManager.visit(oldItems, newItems);
    }
}

BOOST_AUTO_TEST_SUITE_END() 
