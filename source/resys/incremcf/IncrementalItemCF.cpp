#include <idmlib/resys/incremcf/IncrementalItemCF.h>

#include <math.h> // for sqrt

NS_IDMLIB_RESYS_BEGIN

IncrementalItemCF::IncrementalItemCF(
    const std::string& covisit_path, 
    size_t covisit_row_cache_size,
    const std::string& item_item_similarity_path,
    size_t similarity_row_cache_size, 
    const std::string& item_neighbor_path,
    size_t topK,
    const std::string& user_recommendItem_path,
    size_t max_items_stored_for_each_user
)
    : covisitation_(covisit_path, covisit_row_cache_size)
    , similarity_(item_item_similarity_path,similarity_row_cache_size,item_neighbor_path,topK)
    , userRecommendItems_(user_recommendItem_path)
    , max_items_stored_for_each_user_(max_items_stored_for_each_user)
{
}

IncrementalItemCF::~IncrementalItemCF()
{
}

void IncrementalItemCF::batchBuild()
{
    ///batchBuild is not used currently
    //while(userIterator.hasNext())
    //{
	///build covisit
    //}
    ///build similarity
    ///build neightor
    //userIterator.reset();
    //while(userIterator.hasNext())
    //{
       ///store recommenditems for user
    //}
}

void IncrementalItemCF::incrementalBuild(
    uint32_t userId, 
    std::list<uint32_t>& oldItems, 
    std::list<uint32_t>& newItems,
    ItemIterator& itemIterator,
    ItemRescorer* rescorer
)
{
    covisitation_.visit(oldItems, newItems);
    ///oldItems has already contained elements from newItems now
    for(std::list<uint32_t>::iterator it_i = oldItems.begin(); it_i !=oldItems.end(); ++it_i)
    {
        uint32_t Int_I_I = covisitation_.coeff(*it_i, *it_i);
        std::list<std::pair<uint32_t, double> > newValues;
        for(std::list<uint32_t>::iterator it_j = oldItems.begin(); it_j !=oldItems.end(); ++it_j)
        {
            if(*it_j == *it_i) continue;
            uint32_t Int_J_J = covisitation_.coeff(*it_j, *it_j);
            uint32_t Int_I_J = covisitation_.coeff(*it_i, *it_j);
            double denominator = sqrt(Int_I_I) * sqrt(Int_J_J);
            double sim = (denominator == 0)? 0: (double)Int_I_J/denominator;
            similarity_.coeff(*it_i,*it_j,sim);
            newValues.push_back(std::make_pair(*it_j, sim));
        }
        similarity_.adjustNeighbor(*it_i, newValues);
    }
    RecommendItemType recommendItem;
    while(itemIterator.hasNext())
    {
        uint32_t itemId = itemIterator.next();
        if(!rescorer || !rescorer->isFiltered(itemId))
        {
            double preference = estimate(itemId,  oldItems);
            if(preference > 0)
                recommendItem.push_back(std::make_pair(itemId,preference));
        }
    }
    std::sort(recommendItem.begin(), recommendItem.end(),similarityCompare<ItemType,MeasureType>);
    if(recommendItem.size() > max_items_stored_for_each_user_) 
        recommendItem.resize(max_items_stored_for_each_user_);
    userRecommendItems_.setRecommendItem(userId, recommendItem);
}

double IncrementalItemCF::estimate(
    uint32_t itemId, 
    std::list<uint32_t>& itemIds
)
{
    double totalSimilarity = 0.0;
    double preference = 0.0;
    int count = 0;

    std::map<uint32_t, double> similarities;
    totalSimilarity = similarity_.itemSimilarities(itemId, similarities);
    std::list<uint32_t>::iterator it = itemIds.begin();
    for (; it != itemIds.end(); ++it) 
    {
        std::map<uint32_t, double>::iterator iter = similarities.find(*it);
        if(iter != similarities.end())
        {
            double theSimilarity = iter->second;
            if (theSimilarity !=0) 
            {
                preference += theSimilarity;
                count++;
            }
        }
    }
    if(count == 0) return 0;
    return totalSimilarity / preference;
}

void IncrementalItemCF::getTopItems(
    int howMany,
    std::vector<uint32_t>& itemIds,
    std::list<RecommendedItem>& topItems
)
{
    for(size_t i = 0; i < itemIds.size(); ++i)
    {
        std::vector<std::pair<ItemType, MeasureType> > similarities;
        if(similarity_.itemSimilarity(itemIds[i], similarities))
        {
            ///most silly policies
            std::vector<std::pair<ItemType, MeasureType> >::iterator iter;
            for(iter = similarities.begin(); iter != similarities.end(); ++iter)
                topItems.push_back(RecommendedItem(iter->first, iter->second));
        }
    }
    if(topItems.size() > (size_t)howMany) topItems.resize(howMany);
}

void IncrementalItemCF::getTopItems(
    int howMany, 
    uint32_t userId,
    std::list<RecommendedItem>& topItems, 
    std::set<uint32_t>& filterItems
)
{
    RecommendItemType recommendItem;
    if(userRecommendItems_.getRecommendItem(userId, recommendItem))
    {
        int count = 0;
        for(size_t i = 0; i < recommendItem.size(); ++i)
        {
            std::pair<uint32_t, double>& item = recommendItem[i];
            if(filterItems.find(item.first) == filterItems.end())
            {
                topItems.push_back(RecommendedItem(item.first, item.second));
                if(++count >= howMany)
                    break;
            }
        }
    }
}

bool IncrementalItemCF::getUserRecommendItems(
    uint32_t userId, 
    RecommendItemType& results
)
{
    return userRecommendItems_.getRecommendItem(userId, results);
}

void IncrementalItemCF::gc()
{
    covisitation_.gc();
    similarity_.gc();
}

NS_IDMLIB_RESYS_END

