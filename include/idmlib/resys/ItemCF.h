#ifndef IDMLIB_RESYS_ITEM_CF_H
#define IDMLIB_RESYS_ITEM_CF_H

#include <idmlib/idm_types.h>

#include <list>
#include <vector>

NS_IDMLIB_RESYS_BEGIN

class ItemCF
{
public:
    virtual ~ItemCF(){}

    virtual void build(std::list<uint32_t>& oldItems, std::list<uint32_t>& newItems) = 0;

    virtual double estimate(uint32_t itemId, std::vector<uint32_t>& itemIds) =0;

    virtual void gc() = 0;
};

NS_IDMLIB_RESYS_END

#endif

