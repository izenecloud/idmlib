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

