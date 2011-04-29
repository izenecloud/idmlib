#include <idmlib/resys/TopItems.h>

#include <cmath>

NS_IDMLIB_RESYS_BEGIN

TopItems::TopItems(ItemCF* itemCF)
    :itemCF_(itemCF)
{
}

void TopItems::getTopItems(
    int howMany, 
    std::list<RecommendedItem>& topItems, 
    std::list<uint32_t>& becauseOfItems,    
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
            float preference = itemCF_->estimate(itemId,  becauseOfItems);
            queue.insert(RecommendedItem(itemId,preference));
        }
    }
    size_t count = queue.size();
    for(size_t i = 0; i < count; ++i)
    {
	topItems.push_back(RecommendedItem(queue.pop()));
    }
}

NS_IDMLIB_RESYS_END

