#ifndef IDMLIB_RESYS_INCREMENTAL_ITEM_CF_H
#define IDMLIB_RESYS_INCREMENTAL_ITEM_CF_H

#include <idmlib/idm_types.h>

#include <idmlib/resys/ItemCF.h>
#include <idmlib/resys/ItemRescorer.h>
#include <idmlib/resys/RecommendItem.h>
#include <idmlib/resys/ItemCoVisitation.h>
#include <idmlib/resys/SimilarityNeighbor.h>

#include <am/matrix/matrix_db.h>

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

    friend class ItemCFTest;

public:
    IncrementalItemCF(
        const std::string& covisit_path,
        size_t covisit_row_cache_size,
        const std::string& item_item_similarity_path,
        size_t similarity_row_cache_size,
        const std::string& item_neighbor_path,
        size_t topK
    );

    ~IncrementalItemCF();

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
        ItemRescorer* rescorer = NULL
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
        ItemRescorer* rescorer = NULL
    );

    void flush();

    void print(std::ostream& ostream) const;

private:
    /**
     * Update @p row in similarity matrix,
     * the similarity value is calculated based on covisit value @p coVisitRow.
     * @param row the row id to update
     * @param coVisitRow the covisit values of the row
     * @param isUpdateCol if true, for each column in @p coVisitRow, if the column does not exist in @p rowSet,
     *                    also update (col, row) in similarity matrix, and insert this column into @p colSet.
     * @param rowSet only used when @p isUpdateCol is true, to check whether the column exist in this set
     * @param colSet only used when @p isUpdateCol is true, store the columns not exit in @p rowSet
     */
    void updateSimRow_(
        uint32_t row,
        const VisitRow& coVisitRow,
        bool isUpdateCol,
        const std::set<uint32_t>& rowSet,
        std::set<uint32_t>& colSet
    );

    /**
     * Update similarity matrix.
     * @param rows the row numbers to update
     */
    void updateSimMatrix_(const std::list<uint32_t>& rows);
};

std::ostream& operator<<(std::ostream& out, const IncrementalItemCF& increItemCF);

NS_IDMLIB_RESYS_END

#endif
