#ifndef IDMLIB_RESYS_TOPITEM_H
#define IDMLIB_RESYS_TOPITEM_H

#include "ItemIterator.h"
#include "ItemRescorer.h"
#include "ItemCF.h"

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


NS_IDMLIB_RESYS_END

#endif

