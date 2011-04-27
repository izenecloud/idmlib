#include <idmlib/resys/incremcf/IncrementalItemCF.h>

#include <math.h> // for sqrt

NS_IDMLIB_RESYS_BEGIN

IncrementalItemCF::IncrementalItemCF(
    const std::string& covisitPath, 
    size_t covisit_row_cache_size,
    const std::string& similarityPath,
    size_t similarity_row_cache_size, 
    size_t similar_items_cache_size,
    size_t topK
)
    : covisitation_(covisitPath, covisit_row_cache_size)
    , similarity_(similarityPath,similarity_row_cache_size,similar_items_cache_size,topK)
{
}

IncrementalItemCF::~IncrementalItemCF()
{
}

void IncrementalItemCF::build(
    std::list<uint32_t>& oldItems, 
    std::list<uint32_t>& newItems
)
{
    covisitation_.visit(oldItems, newItems);
    ///oldItems has already contained elements from newItems now
    for(std::list<uint32_t>::iterator it_i = oldItems.begin(); it_i !=oldItems.end(); ++it_i)
    {
        uint32_t Int_I_I = covisitation_.coeff(*it_i, *it_i);
        for(std::list<uint32_t>::iterator it_j = oldItems.begin(); it_j !=oldItems.end(); ++it_j)
        {
            if(*it_j == *it_i) continue;
            uint32_t Int_J_J = covisitation_.coeff(*it_j, *it_j);
            uint32_t Int_I_J = covisitation_.coeff(*it_i, *it_j);
            double denomitor = sqrt(Int_I_I) * sqrt(Int_J_J);
            double sim = (denomitor == 0)? 0: (double)Int_I_J/denomitor;
            similarity_.coeff(*it_i,*it_j,sim);
        }
    }
}

double IncrementalItemCF::estimate(
    uint32_t itemId, 
    std::vector<uint32_t>& itemIds
)
{
    double totalSimilarity = 0.0;
    int count = 0;

    std::vector<std::pair<uint32_t, double> > similarities;
    similarity_.itemSimilarities(itemId, itemIds, similarities);
    for (size_t i = 0; i < similarities.size(); i++) 
    {
        double theSimilarity = similarities[i].second;
        if (theSimilarity !=0) 
        {
            // Weights can be negative!
            totalSimilarity += theSimilarity;
            count++;
      }
    }
    if(count == 0) return 0;
    return totalSimilarity / count;
}

void IncrementalItemCF::gc()
{
    covisitation_.gc();
    similarity_.gc();
}

NS_IDMLIB_RESYS_END

