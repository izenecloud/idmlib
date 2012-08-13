#ifndef IDMLIB_RESYS_INCREMENTAL_ITEM_CF_H
#define IDMLIB_RESYS_INCREMENTAL_ITEM_CF_H

#include "ItemCFParam.h"
#include <idmlib/idm_types.h>

#include <idmlib/resys/ItemCF.h>
#include <idmlib/resys/ItemRescorer.h>
#include <idmlib/resys/RecommendItem.h>
#include <idmlib/resys/ItemCoVisitation.h>
#include <idmlib/resys/SimilarityNeighbor.h>
#include <idmlib/resys/MatrixSharder.h>

#include <am/matrix/matrix_db.h>
#include <sdb/SequentialDB.h>

#include <vector>
#include <set>
#include <string>
#include <iostream>

NS_IDMLIB_RESYS_BEGIN

class IncrementalItemCF : public ItemCF
{
private:
    typedef ItemCoVisitation<CoVisitFreq> CoVisitation;
    CoVisitation covisitation_;

    typedef CoVisitation::MatrixType VisitMatrix;
    typedef VisitMatrix::row_type VisitRow;
    VisitMatrix& visitMatrix_;

    typedef izenelib::am::MatrixDB<uint32_t, float> SimMatrix;
    typedef SimMatrix::row_type SimRow;
    SimMatrix simMatrix_;

    typedef SimilarityNeighbor<uint32_t, float> SimNeighbor;
    SimNeighbor simNeighbor_;

    // itemid -> visit freq
    typedef izenelib::sdb::unordered_sdb_tc<uint32_t, uint32_t, ReadWriteLock> VisitFreqDB;
    VisitFreqDB visitFreqDB_;

    MatrixSharder* matrixSharder_;

    class UpdateSimFunc;
    friend class ItemCFTest;

public:
    IncrementalItemCF(
        const ItemCFParam& itemCFParam,
        MatrixSharder* matrixSharder = NULL
    );

    ~IncrementalItemCF();

    void flush();

    void print(std::ostream& ostream) const;

    /**
     * Update covisist and similarity matrix.
     * @param oldItems the items visited before
     * @param newItems the new visited items
     *
     * As the covisit matrix records the number of users who have visited both item i and item j,
     * it has below pre-conditions:
     * @pre each items in @p oldItems should be unique
     * @pre each items in @p newItems should be unique
     * @pre there should be no items contained in both @p oldItems and @p newItems
     */
    void updateMatrix(
        const std::list<uint32_t>& oldItems,
        const std::list<uint32_t>& newItems
    );

    /**
     * Update covisist matrix.
     * @param oldItems the items visited before
     * @param newItems the new visited items
     */
    void updateVisitMatrix(
        const std::list<uint32_t>& oldItems,
        const std::list<uint32_t>& newItems
    );

    /**
     * Rebuild the whole similarity matrix in batch mode.
     * For each row in @c visitMatrix_, to calculate:
     * 1. the similarity value for each column
     * 2. the top K nieghbors for the row
     */
    void buildSimMatrix();

    /**
     * Given @p visitItems, recommend @p recItems.
     * @param howMany the max number of items to recommend
     * @param visitItems the items already visited by user
     * @param recItems the recommend result
     * @param rescorer filter items
     */
    void recommend(
        int howMany,
        const std::vector<uint32_t>& visitItems,
        RecommendItemVec& recItems,
        const ItemRescorer* rescorer = NULL
    );

    /**
     * Given @p visitItemWeights, recommend @p recItems.
     * @param howMany the max number of items to recommend
     * @param visitItemWeights the items already visited by user,
     * each with a weight value as the extent user likes the item,
     * e.g. weight "-1" means dislike, "1" means like, "1.5" means love, etc
     * @param recItems the recommend result
     * @param rescorer filter items
     */
    typedef std::map<uint32_t, float> ItemWeightMap;
    void recommend(
        int howMany,
        const ItemWeightMap& visitItemWeights,
        RecommendItemVec& recItems,
        const ItemRescorer* rescorer = NULL
    );

    /**
     * Get candidate items for recommendation.
     */
    void getCandidateSet(
        const std::vector<uint32_t>& visitItems,
        std::set<uint32_t>& candidateSet
    );

    /**
     * Get candidate items for recommendation.
     * @param visitItemWeights only the items with positive weight would be used
     *                         to get candidate items.
     */
    void getCandidateSet(
        const ItemWeightMap& visitItemWeights,
        std::set<uint32_t>& candidateSet
    );

    /**
     * Given @p candidateSet, recommend @p recItems.
     */
    void recommendFromCandidateSet(
        int howMany,
        const std::vector<uint32_t>& visitItems,
        const std::set<uint32_t>& candidateSet,
        RecommendItemVec& recItems,
        const ItemRescorer* rescorer
    );

    /**
     * Given @p candidateSet, recommend @p recItems.
     */
    void recommendFromCandidateSet(
        int howMany,
        const ItemWeightMap& visitItemWeights,
        const std::set<uint32_t>& candidateSet,
        RecommendItemVec& recItems,
        const ItemRescorer* rescorer
    );

private:
    /**
     * Update similarity matrix, no symmetric value is updated.
     * @param rows the rows to update
     */
    void updateSimMatrix_(const std::list<uint32_t>& rows);

    /**
     * Update similarity matrix, only update the columns @p cols.
     * @param rows the rows to update
     * @param cols only update the columns in @p rows
     */
    void updateSimMatrix_(
        const std::list<uint32_t>& rows,
        const std::list<uint32_t>& cols
    );

    /**
     * Update similarity matrix, also update symmetric values.
     * @param rows the rows to update
     */
    void updateSymmetricMatrix_(const std::list<uint32_t>& rows);

    /**
     * Update @p row in similarity matrix.
     * @param row the row id to update
     * @param coVisitRow the covisit values on the row, used to calculate similarity value
     * @param func a function object, used to update symmetric values in similarity matrix,
     *             it could be NULL when it's not necessary to upate symmetric values.
     */
    void updateSimRow_(
        uint32_t row,
        const VisitRow& coVisitRow,
        UpdateSimFunc* func = NULL
    );

    /**
     * Update the @p cols of @p row in similarity matrix.
     * @param row the row id to update
     * @param coVisitRow the covisit values on the row, used to calculate similarity value
     * @param cols only update similarity matrix on these columns
     * @param func a function object, used to update symmetric values
     */
    void updateSimRowCols_(
        uint32_t row,
        const VisitRow& coVisitRow,
        const std::list<uint32_t>& cols,
        UpdateSimFunc& func
    );

    /**
     * Increment visit frequency.
     */
    void updateVisitFreq_(const std::list<uint32_t>& newItems);

    /**
     * check whether the row should be stored on current node.
     * @param row the row id
     * @return true for store, false for not store
     */
    bool isMyRow_(uint32_t row)
    {
        if (! matrixSharder_)
            return true;

        return matrixSharder_->testRow(row);
    }

    /**
     * a help function to get covisit freq value.
     * @param visitRow it stores all freq values of the row
     * @param col which column to get
     * @param freq it stores freq value of @p col
     * @return true for success, false for @p col has no freq value
     */
    bool getCoVisitFreq_(
        const VisitRow& visitRow,
        uint32_t col,
        uint32_t& freq
    ) const
    {
        VisitRow::const_iterator findIt = visitRow.find(col);
        if (findIt != visitRow.end())
        {
            freq = findIt->second.freq;
            return true;
        }
        return false;
    }
};

std::ostream& operator<<(std::ostream& out, const IncrementalItemCF& increItemCF);

/** function object to update similarity value and neighbor */
class IncrementalItemCF::UpdateSimFunc
{
public:
    UpdateSimFunc(
        SimMatrix& simMatrix,
        SimNeighbor& simNeighbor
    );

    UpdateSimFunc(
        SimMatrix& simMatrix,
        SimNeighbor& simNeighbor,
        const std::list<uint32_t>& excludeRowList
    );

    void operator()(
        uint32_t row,
        uint32_t col,
        float sim
    );

    void updateNeighbor();

private:
    SimMatrix& simMatrix_;
    SimNeighbor& simNeighbor_;

    std::set<uint32_t> excludeRows_;
    std::set<uint32_t> updatedRows_;
};

NS_IDMLIB_RESYS_END

#endif
