#ifndef IDMLIB_RESYS_ITEM_CF_H
#define IDMLIB_RESYS_ITEM_CF_H

#include <idmlib/idm_types.h>

NS_IDMLIB_RESYS_BEGIN

class ItemCF
{
public:
    virtual ~ItemCF(){}

    virtual void flush() = 0;
};

NS_IDMLIB_RESYS_END

#endif

