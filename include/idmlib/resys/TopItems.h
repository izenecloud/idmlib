#ifndef IDMLIB_RESYS_TOPITEM_H
#define IDMLIB_RESYS_TOPITEM_H

#include "RecommendedItem.h"
#include "ItemIterator.h"
#include "ItemRescorer.h"
#include "ItemCF.h"

#include <util/PriorityQueue.h>

#include <boost/shared_ptr.hpp>

#include <list>
#include <vector>

NS_IDMLIB_RESYS_BEGIN

class TopItems
{
public:
    TopItems(ItemCF* itemCF);

    void getTopItems(
        int howMany, 
        std::list<RecommendedItem>& topItems, 
        std::list<uint32_t>& becauseOfItems,
        ItemIterator& itemIterator,
        ItemRescorer* rescorer = NULL
    );
private:
    ItemCF* itemCF_;
};

class TopItemsQueue
    :public izenelib::util::PriorityQueue<RecommendedItem >
{
public:
    TopItemsQueue(size_t size)
    {
        this->initialize(size);
    }
protected:
    bool lessThan(RecommendedItem o1, RecommendedItem o2)
    {
        return (o1.value < o2.value);
    }
};


NS_IDMLIB_RESYS_END

#endif

