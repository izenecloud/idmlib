/**
 * @file ItemCFTest.h
 * @brief utility functions to test methods of @c ItemCoVisitation and @c IncrementalItemCF.
 * @author Jun Jiang
 * @date 2011-06-16
 */

#ifndef ITEM_CF_TEST_H
#define ITEM_CF_TEST_H

#include <idmlib/resys/incremcf/IncrementalItemCF.h>
#include <idmlib/resys/ItemCoVisitation.h>

#include <vector>
#include <list>

NS_IDMLIB_RESYS_BEGIN

class ItemCFTest
{
public:
    typedef ItemCoVisitation<CoVisitFreq> CoVisitation;
    typedef izenelib::am::MatrixDB<uint32_t,float> SimMatrix;

    ItemCFTest()
    : cfManager_(NULL)
    , covisitation_(NULL)
    , simMatrix_(NULL)
    , goldVisitMatrix_(ITEM_NUM, std::vector<int>(ITEM_NUM))
    , goldSimMatrix_(ITEM_NUM, std::vector<float>(ITEM_NUM))
    {}

    void setCFManager(IncrementalItemCF* cfManager)
    {
        cfManager_ = cfManager;
        covisitation_ = &cfManager->covisitation_;
        simMatrix_ = &cfManager_->simMatrix_;
    }

    void setCoVisitation(CoVisitation* covisit)
    {
        covisitation_ = covisit;
    }

    enum {
        /**
         * the max number of items allowed in each test cases,
         * the valid range of item id is [0, ITEM_NUM).
         */
        ITEM_NUM = 10
    };

    /**
     * it calls @c ItemCoVisitation::visit(), and checks each value in visit matrix.
     */
    void checkVisit(
        const char* oldItemStr,
        const char* newItemStr
    );

    /**
    * it calls @c ItemCoVisitation::getCoVisitation() for each item i,
    * and compares them with the non-zero items in @c goldVisitMatrix_[i].
    */
    void checkCoVisitResult();

    /**
     * it calls @c IncrementalItemCF::updateMatrix(), and checks each value in visit matrix and similarity matrix,
     * it also checks each recommended item.
     * @param rebuildSimMatrix if true, IncrementalItemCF::buildSimMatrix() is called,
     *                         if false, IncrementalItemCF::updateVisitMatrix() is called instead.
     */
    void checkPurchase(
        uint32_t userId,
        const char* oldItemStr,
        const char* newItemStr,
        bool rebuildSimMatrx = false
    );

    /**
     * Compare each value in @c covisitation_ and @c goldVisitMatrix_.
     */
    void checkVisitMatrix();

    /**
     * Compare each value in @c simMatrix_ and @c goldSimMatrix_.
     */
    void checkSimMatrix();

    /**
     * it checks recommendation results for @p inputItemStr.
     */
    void checkRecommend(const char* inputItemStr) const;

    /**
     * it checks recommendation results for @p itemWeightPairs.
     */
    void checkWeightRecommend(const char* itemWeightPairs) const;

private:
    /**
     * For each pairs in @p oldItems and @p newItems,
     * increment the value in @c goldVisitMatrix_.
     */
    void updateGoldVisitMatrix_(
        const std::list<uint32_t>& oldItems,
        const std::list<uint32_t>& newItems
    );

    /**
     * For each value in @c goldVisitMatrix_,
     * calculate the similarity value in @c goldSimMatrix_.
     * @note as the similarity value on diagonal line would not be used,
     *       it is assumed as zero.
     */
    void updateGoldSimMatrix_();

    /**
    * it calls @c ItemCoVisitation::getCoVisitation() for @p inputItem,
    * and compare them with the non-zero items in @c goldVisitMatrix_[inputItem].
    */
    void checkCoVisitResult_(uint32_t inputItem);

    void calcCoVisitWeights_(
        uint32_t inputItem,
        std::vector<float>& weights
    ) const;

    /**
     * Check recommend results, given @p inputItems,
     * calculate weight for each item, and compare them with results.
     */
    void checkRecommendItem_(const std::vector<uint32_t>& inputItems) const;

    void calcWeights_(
        const std::vector<uint32_t>& inputItems,
        std::vector<float>& weights
    ) const;

    void calcWeights_(
        const IncrementalItemCF::ItemWeightMap& inputItemWeights,
        std::vector<float>& weights
    ) const;

    void checkWeights_(
        const RecommendItemVec& recItems,
        std::vector<float>& goldWeights
    ) const;

private:
    IncrementalItemCF* cfManager_;
    CoVisitation* covisitation_;
    SimMatrix* simMatrix_;

    std::vector<std::vector<int> > goldVisitMatrix_;
    std::vector<std::vector<float> > goldSimMatrix_;
};

NS_IDMLIB_RESYS_END

#endif // ITEM_CF_TEST_H
