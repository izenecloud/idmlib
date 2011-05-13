#ifndef IDMLIB_RESYS_RECOMMEND_ITEM_H
#define IDMLIB_RESYS_RECOMMEND_ITEM_H

#include <idmlib/idm_types.h>

NS_IDMLIB_RESYS_BEGIN

struct RecommendedItem
{
    RecommendedItem(uint32_t itemId = 0, float value = 0)
        :itemId(itemId)
        ,value(value)
    {}

    RecommendedItem(const RecommendedItem& other)
        :itemId(other.itemId)
        ,value(other.value)
    {}


    uint32_t itemId;
    float value;
};

NS_IDMLIB_RESYS_END

#endif

