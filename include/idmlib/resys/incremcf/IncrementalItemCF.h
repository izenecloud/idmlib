#ifndef IDMLIB_RESYS_INCREMENTAL_ITEM_CF_H
#define IDMLIB_RESYS_INCREMENTAL_ITEM_CF_H

#include <idmlib/idm_types.h>

#include <idmlib/resys/ItemCF.h>
#include <idmlib/resys/ItemCoVisitation.h>
#include <idmlib/resys/similarity/SimilarityMatrix.h>

#include <vector>
#include <string>

NS_IDMLIB_RESYS_BEGIN

class IncrementalItemCF : public ItemCF
{
public:
    IncrementalItemCF(
        const std::string& covisitPath, 
        size_t covisit_row_cache_size,
        const std::string& similarityPath,
        size_t similarity_row_cache_size, 
        size_t similar_items_cache_size,
        size_t topK = 30
        );

    ~IncrementalItemCF();

public:
    void build(std::list<uint32_t>& oldItems, std::list<uint32_t>& newItems);

    double estimate(uint32_t itemId, std::vector<uint32_t>& itemIds);

    void gc();

private:
    ItemCoVisitation<CoVisitFreq> covisitation_;
    SimilarityMatrix<uint32_t,double> similarity_;

};

NS_IDMLIB_RESYS_END

#endif
