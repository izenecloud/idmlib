#ifndef IDMLIB_RESYS_ITEMRESCORER_H
#define IDMLIB_RESYS_ITEMRESCORER_H

#include <idmlib/idm_types.h>

NS_IDMLIB_RESYS_BEGIN

struct ItemRescorer
{
    virtual ~ItemRescorer(){}

    virtual float rescore(uint32_t itemId, float originalScore) = 0;

    virtual bool isFiltered(uint32_t itemId) const = 0;
};

NS_IDMLIB_RESYS_END

#endif

