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
    std::stringstream ss(inputStr);
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

    visitMatrix_->visit(oldItems, newItems);

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
            BOOST_CHECK_EQUAL(static_cast<int>(visitMatrix_->coeff(i, j)),
                              goldVisitMatrix_[i][j]);
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
    const char* newItemStr,
    bool rebuildSimMatrx
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

    if (rebuildSimMatrx)
    {
        cfManager_->updateVisitMatrix(oldItems, newItems);
        cfManager_->buildSimMatrix();
    }
    else
    {
        cfManager_->updateMatrix(oldItems, newItems);
    }

    checkVisitMatrix();
    updateGoldSimMatrix_();
    checkSimMatrix();

    std::vector<uint32_t> totalItems(oldItems.begin(), oldItems.end());
    totalItems.insert(totalItems.end(), newItems.begin(), newItems.end());

    checkRecommendItem_(totalItems);
}

void ItemCFTest::checkRecommend(const char* inputItemStr) const
{
    std::vector<uint32_t> inputItems;
    splitItems(inputItemStr, back_insert_iterator< vector<uint32_t> >(inputItems));

    checkRecommendItem_(inputItems);
}

void ItemCFTest::checkRecommendItem_(const std::vector<uint32_t>& inputItems) const
{
    // get recommend items
    RecommendItemVec recItems;
    cfManager_->recommend(ITEM_NUM, inputItems, recItems);

    /*std::list<uint32_t> recItemIDs;
    for (RecommendItemVec::const_iterator it = recItems.begin();
        it != recItems.end(); ++it)
    {
        recItemIDs.push_back(it->itemId_);
    }
    cout << "\t<= given items ";
    copy(inputItems.begin(), inputItems.end(), COUT_IT);
    cout << "recommend other items: ";
    copy(recItemIDs.begin(), recItemIDs.end(), COUT_IT);
    cout << endl;*/

    std::vector<float> goldWeights;
    calcWeights_(inputItems, goldWeights);

    checkWeights_(recItems, goldWeights);
}

void ItemCFTest::calcWeights_(
    const std::vector<uint32_t>& inputItems,
    std::vector<float>& weights
) const
{
    weights.assign(ITEM_NUM, 0);

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
            weights[i] = sim / total;
        }
    }
}

void ItemCFTest::checkWeights_(
    const RecommendItemVec& recItems,
    std::vector<float>& goldWeights
) const
{
    // check weight in descreasing order
    float prevWeight = FLT_MAX;
    for (RecommendItemVec::const_iterator it = recItems.begin();
        it != recItems.end(); ++it)
    {
        uint32_t itemId = it->itemId_;
        float weight = it->weight_;

        //cout << "item: " << itemId << ", weight: " << weight << endl;
        BOOST_CHECK_CLOSE(weight, goldWeights[itemId], 0.0001);
        BOOST_CHECK(weight <= prevWeight);

        // reset weight value to compare with zeroWeightVec
        goldWeights[itemId] = 0;
        prevWeight = weight;
    }

    // to check all results are returned
    std::vector<float> zeroWeightVec(ITEM_NUM);
    BOOST_CHECK_EQUAL_COLLECTIONS(goldWeights.begin(), goldWeights.end(),
                                  zeroWeightVec.begin(), zeroWeightVec.end());
}

void ItemCFTest::checkWeightRecommend(const char* itemWeightPairs) const
{
    IncrementalItemCF::ItemWeightMap itemWeightMap;

    idmlib::recommender::ItemType itemId;
    float weight;
    std::stringstream ss(itemWeightPairs);
    while (ss >> itemId >> weight)
    {
        itemWeightMap[itemId] = weight;
        //std::cout << itemId << ", " << weight << std::endl;
    }

    // get recommend items
    RecommendItemVec recItems;
    cfManager_->recommend(ITEM_NUM, itemWeightMap, recItems);

    std::vector<float> goldWeights;
    calcWeights_(itemWeightMap, goldWeights);

    checkWeights_(recItems, goldWeights);
}

void ItemCFTest::calcWeights_(
    const IncrementalItemCF::ItemWeightMap& inputItemWeights,
    std::vector<float>& weights
) const
{
    weights.assign(ITEM_NUM, 0);

    for (unsigned int i=0; i<ITEM_NUM; ++i)
    {
        if (inputItemWeights.find(i) != inputItemWeights.end())
            continue;

        float sim = 0;
        float total = 0;
        for (unsigned int j=0; j<ITEM_NUM; ++j)
        {
            if (i == j)
                continue;

            IncrementalItemCF::ItemWeightMap::const_iterator findIt = inputItemWeights.find(j);
            if (findIt != inputItemWeights.end())
                sim += goldSimMatrix_[i][j] * findIt->second;

            total += goldSimMatrix_[i][j];
        }

        if (sim != 0)
        {
            assert(total != 0);
            float w = sim / total;
            if (w > 0)
                weights[i] = w;
        }
    }
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

NS_IDMLIB_RESYS_END
