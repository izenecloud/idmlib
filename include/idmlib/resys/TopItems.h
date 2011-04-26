#ifndef IDMLIB_RESYS_TOPITEM_H
#define IDMLIB_RESYS_TOPITEM_H

#include "RecommendedItem.h"
#include "ItemIterator.h"
#include "ItemRescorer.h"

#include "similarity/SimilarityMatrix.h"

#include <util/PriorityQueue.h>

#include <list>
#include <vector>

NS_IDMLIB_RESYS_BEGIN

class TopItems
{
public:
    TopItems();

    void getTopItems(
        int howMany, 
        std::list<RecommendedItem>& topItems, 
        std::vector<uint32_t>& becauseOfItems,
        ItemIterator& itemIterator,
        ItemRescorer* rescorer = NULL
    );
private:
    double estimate(uint32_t itemId, std::vector<uint32_t>& itemIds);

private:
    boost::shared_ptr<SimilarityMatrix<uint32_t,double> > similarity;
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

