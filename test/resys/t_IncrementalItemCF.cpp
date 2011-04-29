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
const char* TEST_DIR_STR = "cf";

ostream_iterator<uint32_t> COUT_IT(cout, ", ");

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
        return filterSet_.find(itemId) == filterSet_.end();
    }

    std::set<uint32_t> filterSet_;
};

}

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
            cfPathStr + "/nb", 30,
            cfPathStr + "/rec", 1000
        );

        // user1 buys items 1, 2, 3
        uint32_t user1 = 1;
        list<uint32_t> oldItems;
        list<uint32_t> newItems;
        newItems.push_back(1);
        newItems.push_back(2);
        newItems.push_back(3);

        const std::size_t totalNum = oldItems.size() + newItems.size();

        cfManager.incrementalBuild(user1, oldItems, newItems, itemIterator);

        // now newItems is moved into oldItems 
        BOOST_CHECK_EQUAL(oldItems.size(), totalNum);
        BOOST_CHECK_EQUAL(newItems.size(), 0);

        // user2 buys items 2, 3, 4
        uint32_t user2 = 2;
        oldItems.clear();
        newItems.push_back(2);
        newItems.push_back(3);
        newItems.push_back(4);

        cfManager.incrementalBuild(user2, oldItems, newItems, itemIterator);

        // it should recommend user2 with item 1
        std::list<RecommendedItem> topItems;
        cfManager.getTopItems(10, user2, topItems);
        BOOST_CHECK_EQUAL(topItems.size(), 1);
        BOOST_CHECK_EQUAL(topItems.front().itemId, 1);

        // given item 1, it should recommend items 2, 3
        topItems.clear();
        std::vector<uint32_t> inputItemIds;
        inputItemIds.push_back(1);
        cfManager.getTopItems(10, inputItemIds, topItems);
        BOOST_CHECK_EQUAL(topItems.size(), 2);
        BOOST_CHECK((topItems.front().itemId == 2 && topItems.back().itemId == 3)
                    || (topItems.front().itemId == 3 && topItems.back().itemId == 2));
    }
}

BOOST_AUTO_TEST_SUITE_END() 
