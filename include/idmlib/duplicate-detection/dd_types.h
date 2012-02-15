#ifndef IDMLIB_DD_DDTYPES_H_
#define IDMLIB_DD_DDTYPES_H_

#include <idmlib/idm_types.h>


NS_IDMLIB_DD_BEGIN

struct NullType
{
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
    }

    bool dd(const NullType& other) const
    {
        return true;
    }

};

NS_IDMLIB_DD_END

#endif /* DUPDETECTOR_H_ */
