#include <idmlib/resys/TopItems.h>

#include <cmath>

NS_IDMLIB_RESYS_BEGIN

TopItems::TopItems()
{
}

void TopItems::getTopItems(
    int howMany, 
    std::list<RecommendedItem>& topItems, 
    std::vector<uint32_t>& becauseOfItems,    
    ItemIterator& itemIterator,
    ItemRescorer* rescorer
)
{
    TopItemsQueue queue(howMany);
    while(itemIterator.hasNext())
    {
        uint32_t itemId = itemIterator.next();
        if(!rescorer || !rescorer->isFiltered(itemId))
        {
            double preference = estimate(itemId,  becauseOfItems);
            queue.insert(RecommendedItem(itemId,preference));
        }
    }
    size_t count = queue.size();
    for(size_t i = 0; i < count; ++i)
    {
	topItems.push_back(RecommendedItem(queue.pop()));
    }
}

double TopItems::estimate(
    uint32_t itemId, 
    std::vector<uint32_t>& itemIds
)
{
    double preference = 0.0;
    double totalSimilarity = 0.0;
    int count = 0;
/*
    std::vector<std::pair<uint32_t, double> > similarities;
    similarity->itemSimilarities(itemId, itemIds, similarities);
    for (int i = 0; i < similarities.size(); i++) {
      double theSimilarity = similarities[i].second;
      if (!std::isnan(theSimilarity)) {
        // Weights can be negative!
        preference += theSimilarity;
        totalSimilarity += theSimilarity;
        count++;
      }
    }
*/
    return preference / totalSimilarity;
}


NS_IDMLIB_RESYS_END

