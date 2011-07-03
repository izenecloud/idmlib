/**
 * @file ItemCFTest.cpp
 * @author Jun Jiang
 * @date 2011-06-17
 */

#include "ItemCFTest.h"

#include <boost/test/unit_test.hpp>

#include <set>
#include <algorithm> // copy
#include <cmath> // sqrt
#include <climits> // INT_MAX
#include <cfloat> // FLT_MAX

namespace
{
std::ostream_iterator<idmlib::recommender::ItemType> COUT_IT(std::cout, ", ");

/**
 * Split string @p inputStr and append each element to iterator @p result.
 */
template <class OutputIterator>
void splitItems(
    const char* inputStr,
    OutputIterator result
)
{
    idmlib::recommender::ItemType itemId;
    stringstream ss(inputStr);
    while (ss >> itemId)
    {
        *result++ = itemId;
    }
}

}

NS_IDMLIB_RESYS_BEGIN

void ItemCFTest::checkVisit(
    const char* oldItemStr,
    const char* newItemStr
)
{
    list<uint32_t> oldItems;
    list<uint32_t> newItems;

    splitItems(oldItemStr, back_insert_iterator< list<uint32_t> >(oldItems));
    splitItems(newItemStr, back_insert_iterator< list<uint32_t> >(newItems));

    /*cout << "=> visit event, old items: ";
    copy(oldItems.begin(), oldItems.end(), COUT_IT);
    cout << ", new items: ";
    copy(newItems.begin(), newItems.end(), COUT_IT);
    cout << endl;*/

    updateGoldVisitMatrix_(oldItems, newItems);

    // update visitMatrix_
    const std::size_t totalNum = oldItems.size() + newItems.size();
    visitMatrix_->visit(oldItems, newItems);
    // now newItems is moved into oldItems 
    BOOST_CHECK_EQUAL(oldItems.size(), totalNum);
    BOOST_CHECK_EQUAL(newItems.size(), 0);

    checkVisitMatrix();
}

void ItemCFTest::updateGoldVisitMatrix_(
    const std::list<uint32_t>& oldItems,
    const std::list<uint32_t>& newItems
)
{
    for (list<uint32_t>::const_iterator newIt = newItems.begin();
        newIt != newItems.end(); ++newIt)
    {
        // update 2*new*old pairs
        for (list<uint32_t>::const_iterator oldIt = oldItems.begin();
            oldIt != oldItems.end(); ++oldIt)
        {
            ++goldVisitMatrix_[*newIt][*oldIt];
            ++goldVisitMatrix_[*oldIt][*newIt];
        }

        // update new*new pairs
        for (list<uint32_t>::const_iterator newIt2 = newItems.begin();
            newIt2 != newItems.end(); ++newIt2)
        {
            ++goldVisitMatrix_[*newIt][*newIt2];
        }
    }
}

void ItemCFTest::checkVisitMatrix()
{
    for (int i=0; i<ITEM_NUM; ++i)
    {
        for (int j=0; j<ITEM_NUM; ++j)
        {
            BOOST_TEST_MESSAGE("check visit coeff [" << i << "][" << j << "]: " << goldVisitMatrix_[i][j]);
            BOOST_CHECK_EQUAL(visitMatrix_->coeff(i, j), goldVisitMatrix_[i][j]);
        }
    }
}

void ItemCFTest::checkCoVisitResult()
{
    for (uint32_t i=0; i<ITEM_NUM; ++i)
    {
        checkCoVisitResult_(i);
    }
}

void ItemCFTest::checkCoVisitResult_(uint32_t inputItem)
{
    // get covisit items
    std::vector<ItemType> resultVec;
    visitMatrix_->getCoVisitation(ITEM_NUM, inputItem, resultVec);

    // check freq in descreasing order
    std::vector<int> goldVec(goldVisitMatrix_[inputItem]);
    goldVec[inputItem] = 0;
    int prevFreq = INT_MAX;
    for (std::vector<ItemType>::const_iterator resultIt = resultVec.begin();
        resultIt != resultVec.end(); ++resultIt)
    {
        int freq = goldVec[*resultIt];
        // check result freq should not be zero
        BOOST_CHECK(freq != 0);

        // check result is sorted by freq decreasingly
        BOOST_CHECK(freq <= prevFreq);

        // reset freq value to compare with zeroVec
        goldVec[*resultIt] = 0;
        prevFreq = freq;
    }

    // to check all results are returned
    std::vector<int> zeroVec(ITEM_NUM);
    BOOST_CHECK_EQUAL_COLLECTIONS(goldVec.begin(), goldVec.end(),
                                  zeroVec.begin(), zeroVec.end());
}

void ItemCFTest::checkPurchase(
    uint32_t userId,
    const char* oldItemStr,
    const char* newItemStr
)
{
    list<uint32_t> oldItems;
    list<uint32_t> newItems;

    splitItems(oldItemStr, back_insert_iterator< list<uint32_t> >(oldItems));
    splitItems(newItemStr, back_insert_iterator< list<uint32_t> >(newItems));

    /*cout << "=> purchase event, userId: " << userId << ", old items: ";
    copy(oldItems.begin(), oldItems.end(), COUT_IT);
    cout << ", new items: ";
    copy(newItems.begin(), newItems.end(), COUT_IT);
    cout << endl;*/

    updateGoldVisitMatrix_(oldItems, newItems);

    const std::size_t totalNum = oldItems.size() + newItems.size();
    cfManager_->buildMatrix(oldItems, newItems);

    // now newItems is moved into oldItems 
    BOOST_CHECK_EQUAL(oldItems.size(), totalNum);
    BOOST_CHECK_EQUAL(newItems.size(), 0);

    checkVisitMatrix();
    updateGoldSimMatrix_();
    checkSimMatrix();

    MyItemIterator itemIterator(1, ItemCFTest::ITEM_NUM);
    cfManager_->buildUserRecommendItems(userId, oldItems, itemIterator);

    // get recommend items
    std::list<RecommendedItem> topItems;
    cfManager_->getTopItems(ITEM_NUM, userId, topItems);

    /*std::list<uint32_t> recItems;
    for (std::list<RecommendedItem>::const_iterator it = topItems.begin();
        it != topItems.end(); ++it)
    {
        recItems.push_back(it->itemId);
    }
    cout << "\t<= recommend to user " << userId << " with items: ";
    copy(recItems.begin(), recItems.end(), COUT_IT);
    cout << endl;*/

    checkUserRecommend_(oldItems, topItems);
}

void ItemCFTest::checkItemRecommend(const char* inputItemStr)
{
    std::list<uint32_t> inputItems;
    splitItems(inputItemStr, back_insert_iterator< list<uint32_t> >(inputItems));

    // get recommend items
    std::list<RecommendedItem> topItems;
    std::vector<uint32_t> inputVec(inputItems.begin(), inputItems.end());
    cfManager_->getTopItems(ITEM_NUM, inputVec, topItems);

    std::list<uint32_t> recItems;
    for (std::list<RecommendedItem>::const_iterator it = topItems.begin();
        it != topItems.end(); ++it)
    {
        recItems.push_back(it->itemId);
    }

    /*cout << "\t<= given items ";
    copy(inputItems.begin(), inputItems.end(), COUT_IT);
    cout << "recommend other items: ";
    copy(recItems.begin(), recItems.end(), COUT_IT);
    cout << endl;*/

    checkBABResult_(inputItems, recItems);
}

void ItemCFTest::updateGoldSimMatrix_()
{
    for (int i=0; i<ITEM_NUM; ++i)
    {
        for (int j=0; j<ITEM_NUM; ++j)
        {
            float gold = 0;
            if (i != j && goldVisitMatrix_[i][j] != 0)
            {
                gold = goldVisitMatrix_[i][j] / (sqrt(goldVisitMatrix_[i][i]) * sqrt(goldVisitMatrix_[j][j]));
            }
            goldSimMatrix_[i][j] = gold;
        }
    }
}

void ItemCFTest::checkSimMatrix()
{
    for (int i=0; i<ITEM_NUM; ++i)
    {
        for (int j=0; j<ITEM_NUM; ++j)
        {
            BOOST_TEST_MESSAGE("check similarity coeff [" << i << "][" << j << "]: " << goldSimMatrix_[i][j]);
            BOOST_CHECK_CLOSE(simMatrix_->coeff(i, j), goldSimMatrix_[i][j], 0.000001);
        }
    }
}

void ItemCFTest::checkBABResult_(
    const std::list<uint32_t>& inputItems,
    const std::list<uint32_t>& recItems
) const
{
    // get covisit items
    vector<bool> goldRecItems(ITEM_NUM);
    for (std::list<uint32_t>::const_iterator it = inputItems.begin();
        it != inputItems.end(); ++it)
    {
        for (unsigned int j=0; j<ITEM_NUM; ++j)
        {
            if (goldVisitMatrix_[*it][j] != 0)
            {
                goldRecItems[j] = true;
            }
        }
    }
    // filter out input items
    for (std::list<uint32_t>::const_iterator it = inputItems.begin();
        it != inputItems.end(); ++it)
    {
        goldRecItems[*it] = false;
    }

    vector<bool> isRecItems(ITEM_NUM);
    for (std::list<uint32_t>::const_iterator it = recItems.begin();
        it != recItems.end(); ++it)
    {
        isRecItems[*it] = true;
    }

    BOOST_CHECK_EQUAL_COLLECTIONS(isRecItems.begin(), isRecItems.end(),
                                  goldRecItems.begin(), goldRecItems.end());
}

void ItemCFTest::checkUserRecommend_(
    const std::list<uint32_t>& inputItems,
    const std::list<RecommendedItem>& recItems
) const
{
    // calculate weight
    std::vector<float> goldWeightVec(ITEM_NUM);
    std::set<uint32_t> inputSet(inputItems.begin(), inputItems.end());
    for (unsigned int i=0; i<ITEM_NUM; ++i)
    {
        if (inputSet.find(i) != inputSet.end())
            continue;

        float sim = 0;
        float total = 0;
        for (unsigned int j=0; j<ITEM_NUM; ++j)
        {
            if (i == j)
                continue;

            if (inputSet.find(j) != inputSet.end())
                sim += goldSimMatrix_[i][j];

            total += goldSimMatrix_[i][j];
        }

        if (sim != 0)
        {
            assert(total != 0);
            goldWeightVec[i] = sim / total;
        }
    }

    // check weight in descreasing order
    float prevWeight = FLT_MAX;
    for (std::list<RecommendedItem>::const_iterator it = recItems.begin();
        it != recItems.end(); ++it)
    {
        uint32_t itemId = it->itemId;
        float weight = it->value;

        //cout << "item: " << itemId << ", weight: " << weight << endl;
        BOOST_CHECK_CLOSE(weight, goldWeightVec[itemId], 0.0001);
        BOOST_CHECK(weight <= prevWeight);

        // reset weight value to compare with zeroWeightVec
        goldWeightVec[itemId] = 0;
        prevWeight = weight;
    }

    // to check all results are returned
    std::vector<float> zeroWeightVec(ITEM_NUM);
    BOOST_CHECK_EQUAL_COLLECTIONS(goldWeightVec.begin(), goldWeightVec.end(),
                                  zeroWeightVec.begin(), zeroWeightVec.end());
}

NS_IDMLIB_RESYS_END
