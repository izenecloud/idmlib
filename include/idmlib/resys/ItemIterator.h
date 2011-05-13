#ifndef IDMLIB_RESYS_ITEM_ITERATOR_H
#define IDMLIB_RESYS_ITEM_ITERATOR_H

#include <idmlib/idm_types.h>

NS_IDMLIB_RESYS_BEGIN

struct ItemIterator
{
    virtual ~ItemIterator(){}

    virtual bool hasNext() = 0;

    virtual uint32_t next() = 0;
};

NS_IDMLIB_RESYS_END

#endif

