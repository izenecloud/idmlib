#ifndef IDMLIB_RESYS_ITEM_CF_H
#define IDMLIB_RESYS_ITEM_CF_H

#include "RecommendedItem.h"
#include <idmlib/idm_types.h>

#include <util/PriorityQueue.h>

#include <list>
#include <vector>

NS_IDMLIB_RESYS_BEGIN

class ItemCF
{
public:
    virtual ~ItemCF(){}

    /*virtual void build(std::list<uint32_t>& oldItems, std::list<uint32_t>& newItems) = 0;*/

    virtual float estimate(uint32_t itemId, const std::list<uint32_t>& itemIds) =0;

    virtual void flush() = 0;
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

