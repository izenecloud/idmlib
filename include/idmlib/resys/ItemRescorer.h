#ifndef IDMLIB_RESYS_ITEMRESCORER_H
#define IDMLIB_RESYS_ITEMRESCORER_H

#include <idmlib/idm_types.h>

NS_IDMLIB_RESYS_BEGIN

struct ItemRescorer
{
    virtual ~ItemRescorer(){}

    virtual double rescore(uint32_t itemId, double originalScore) = 0;

    virtual bool isFiltered(uint32_t itemId) = 0;
};

NS_IDMLIB_RESYS_END

#endif

