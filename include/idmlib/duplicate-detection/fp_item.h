
#ifndef SF1R_DD_FPITEM_H_
#define SF1R_DD_FPITEM_H_


#include "dd_types.h"
#include <string>
#include <vector>
#include <boost/serialization/access.hpp>
#include <boost/serialization/serialization.hpp>
NS_IDMLIB_DD_BEGIN

class FpItem
{
public:
    FpItem():docid(), fp(), length(0), status(0)
    {
    }

    FpItem(const DocIdType& pdocid, const izenelib::util::CBitArray& pfp, uint32_t plength)
            :docid(pdocid), fp(pfp), length(plength), status(0)
    {
    }
    DocIdType docid;
    izenelib::util::CBitArray fp;
    uint32_t length;
    int status; //1 means its new, 0 means old, not serialized.

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & docid & fp & length;
    }
};

NS_IDMLIB_DD_END

#endif
