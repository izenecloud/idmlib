#ifndef IDMLIB_RESYS_RECOMMEND_ITEM_H
#define IDMLIB_RESYS_RECOMMEND_ITEM_H

#include <idmlib/idm_types.h>

#include <vector>
#include <boost/serialization/access.hpp>

NS_IDMLIB_RESYS_BEGIN

struct RecommendItem
{
    RecommendItem(uint32_t itemId = 0, float weight = 0)
        :itemId_(itemId)
        ,weight_(weight)
    {}

    uint32_t itemId_;
    float weight_;

    /** the items having major influence on recommending @c itemId_ */
    std::vector<uint32_t> reasonItemIds_;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & itemId_;
        ar & weight_;
        ar & reasonItemIds_;
    }
};

typedef std::vector<RecommendItem> RecommendItemVec;

NS_IDMLIB_RESYS_END

#endif

