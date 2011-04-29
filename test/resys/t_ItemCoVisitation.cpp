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

void printItems(const list<ItemType>& oldItems, const list<ItemType>& newItems)
{
    cout << "old items: ";
    copy(oldItems.begin(), oldItems.end(), COUT_IT);

    cout << ", new items: ";
    copy(newItems.begin(), newItems.end(), COUT_IT);

    cout << endl;
}

/**
 * it get covisit items for @p inputItem,
 * and compare them with @p totalItems excluding @p inputItem.
 * @param coVisitManager the covisit manager
 * @param totalItems all the items visited by one user
 * @param inputItem the item used as input to get covisit items
 */
void checkCoVisitResult(CoVisitManager& coVisitManager, const list<ItemType>& totalItems, ItemType inputItem)
{
    // get other items
    vector<ItemType> goldResult;
    for (list<ItemType>::const_iterator it = totalItems.begin();
        it != totalItems.end(); ++it)
    {
        if (*it != inputItem)
        {
            goldResult.push_back(*it);
        }
    }
    sort(goldResult.begin(), goldResult.end());

    // get covisit items
    vector<ItemType> result;
    coVisitManager.getCoVisitation(totalItems.size(), inputItem, result);

    cout << "covisit items for item " << inputItem << " is: ";
    copy(result.begin(), result.end(), COUT_IT);
    cout << endl;

    // check covisit items
    sort(result.begin(), result.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(result.begin(), result.end(),
                                  goldResult.begin(), goldResult.end());
}

BOOST_AUTO_TEST_SUITE(ItemCoVisitationTest)

BOOST_AUTO_TEST_CASE(smokeTest)
{
    bfs::path covisitPath(TEST_DIR_STR);
    boost::filesystem::remove_all(covisitPath);
    bfs::create_directories(covisitPath);

    list<ItemType> oldItems;
    list<ItemType> newItems;
    newItems.push_back(1);
    newItems.push_back(2);
    newItems.push_back(3);

    {
        CoVisitManager coVisitManager(covisitPath.string());

        printItems(oldItems, newItems);

        const std::size_t totalNum = oldItems.size() + newItems.size();

        // visit new items
        coVisitManager.visit(oldItems, newItems);

        // now newItems is moved into oldItems 
        BOOST_CHECK_EQUAL(oldItems.size(), totalNum);
        BOOST_CHECK_EQUAL(newItems.size(), 0);

        checkCoVisitResult(coVisitManager, oldItems, 1);
    }

    {
        CoVisitManager coVisitManager(covisitPath.string());
        checkCoVisitResult(coVisitManager, oldItems, 2);

        newItems.push_back(4);
        newItems.push_back(5);
        newItems.push_back(6);

        printItems(oldItems, newItems);

        coVisitManager.visit(oldItems, newItems);
        checkCoVisitResult(coVisitManager, oldItems, 5);

        list<ItemType> oldItems2;
        list<ItemType> newItems2;
        newItems2.push_back(7);
        newItems2.push_back(8);
        newItems2.push_back(9);
        oldItems.insert(oldItems.end(), newItems2.begin(), newItems2.end());

        newItems2.push_back(2);
        newItems2.push_back(5);

        printItems(oldItems2, newItems2);

        coVisitManager.visit(oldItems2, newItems2);
        checkCoVisitResult(coVisitManager, oldItems2, 8);
        checkCoVisitResult(coVisitManager, oldItems, 2);
    }
}

BOOST_AUTO_TEST_SUITE_END() 
