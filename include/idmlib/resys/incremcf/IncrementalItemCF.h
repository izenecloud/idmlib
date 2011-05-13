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

    void incrementalBuild(
        uint32_t userId, 
        std::list<uint32_t>& oldItems, 
        std::list<uint32_t>& newItems, 
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

    float estimate(uint32_t itemId, std::list<uint32_t>& itemIds);

    bool getUserRecommendItems(uint32_t userId, RecommendItemType& results);

    void gc();

private:
    ItemCoVisitation<CoVisitFreq> covisitation_;
    SimilarityMatrix<uint32_t,uint8_t> similarity_;
    UserRecommendItem userRecommendItems_;
    size_t max_items_stored_for_each_user_;
};

NS_IDMLIB_RESYS_END

#endif
