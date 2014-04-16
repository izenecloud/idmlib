#ifndef IDMLIB_B5M_B5MMODE_H_
#define IDMLIB_B5M_B5MMODE_H_
#include "b5m_types.h"

NS_IDMLIB_B5M_BEGIN

struct B5MMode
{
    enum {INC = 0, REBUILD, REMATCH, RETRAIN};
};

NS_IDMLIB_B5M_END

#endif

