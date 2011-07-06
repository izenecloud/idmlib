#ifndef IDMLIB_RESYS_INCREMENTAL_ITEM_CF_H
#define IDMLIB_RESYS_INCREMENTAL_ITEM_CF_H

#include <idmlib/idm_types.h>

#include "UserRecommendItem.h"
#include <idmlib/resys/ItemCF.h>
#include <idmlib/resys/ItemIterator.h>
#include <idmlib/resys/ItemRescorer.h>
#include <idmlib/resys/RecommendedItem.h>
#include <idmlib/resys/ItemCoVisitation.h>
#include <idmlib/resys/similarity/SimilarityMatrix.h>

#include <vector>
#include <set>
#include <string>

NS_IDMLIB_RESYS_BEGIN

class IncrementalItemCF : public ItemCF
{
public:
    IncrementalItemCF(
        const std::string& covisit_path, 
        size_t covisit_row_cache_size,
        const std::string& item_item_similarity_path,
        size_t similarity_row_cache_size, 
        const std::string& item_neighbor_path,
        size_t topK,
        const std::string& user_recommendItem_path,
        size_t max_items_stored_for_each_user = 1000
        );

    ~IncrementalItemCF();

public:
    void batchBuild();

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
     *
     * for performance reason, it has below post-condition:
     * @post when this function returns, the items in @p newItems would be moved to the end of @p oldItems,
     *       and @p newItems would be empty.
     */
    void buildMatrix(
        std::list<uint32_t>& oldItems,
        std::list<uint32_t>& newItems
    );

    /**
     * Build recommend items for user.
     * @param userId the user id
     * @param items the items already visited by the user
     * @param itemIterator iterate all items
     * @param rescorer filter items
     */
    void buildUserRecommendItems(
        uint32_t userId,
        const std::list<uint32_t>& items,
        ItemIterator& itemIterator,
        ItemRescorer* rescorer = NULL
    );

    void getTopItems(
        int howMany, 
        uint32_t userId,
        std::list<RecommendedItem>& topItems, 
        ItemRescorer* rescorer = NULL
    );

    void getTopItems(
        int howMany,
        const std::vector<uint32_t>& itemIds,
        std::list<RecommendedItem>& topItems,
        ItemRescorer* rescorer = NULL
    );

    float estimate(uint32_t itemId, const std::list<uint32_t>& itemIds);

    void flush();

private:
    /**
     * Update covisist matrix.
     * @param oldItems the items visited before
     * @param newItems the new visited items
     *
     * for performance reason, it has below post-condition:
     * @post when this function returns, the items in @p newItems would be moved to the end of @p oldItems,
     *       and @p newItems would be empty.
     */
    void updateVisitMatrix_(
        std::list<uint32_t>& oldItems,
        std::list<uint32_t>& newItems
    );

    /**
     * Update similarity matrix.
     * @param rows the row numbers to update
     */
    void updateSimMatrix_(const std::list<uint32_t>& rows);

    /**
     * Calculate the value of similarity matrix[row][col].
     * @param row the row number of similarity matrix
     * @param col the column number of similarity matrix
     * @return the value for similarity matrix[row][col]
     */
    float calcSimValue_(
        uint32_t row,
        uint32_t col
    );

private:
    ItemCoVisitation<CoVisitFreq> covisitation_;
    SimilarityMatrix<uint32_t,float> similarity_;
    UserRecommendItem userRecommendItems_;
    size_t max_items_stored_for_each_user_;

    friend class ItemCFTest;
};

NS_IDMLIB_RESYS_END

#endif
