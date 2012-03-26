#ifndef IDMLIB_DD_FPITEM_H_
#define IDMLIB_DD_FPITEM_H_

#include "dd_types.h"
#include <string>
#include <vector>


NS_IDMLIB_DD_BEGIN

template <typename DocIdType, class AttachType = NullType>
class FpItem
{
public:
    FpItem() : docid(), length(0), status(0), attach()
    {
    }

    FpItem(const DocIdType& pdocid, uint32_t plength, const FpType& pfp, const AttachType& pattach = AttachType())
        : docid(pdocid), length(plength), status(0), fp(pfp), attach(pattach)
    {
    }

    bool operator<(const FpItem& other) const
    {
        return docid < other.docid;
    }

    uint32_t calcHammingDist(const uint64_t* ofp) const
    {
        uint32_t hamming_dist = 0;

        for (uint32_t i = 0; i < FpType::FP_SIZE; i++)
            hamming_dist += countBits_(fp.desc[i] ^ ofp[i]);

        return hamming_dist;
    }

private:
    uint32_t countBits_(uint64_t v) const
    {
        v -= (v >> 1) & 0x5555555555555555ULL;
        v = (v & 0x3333333333333333ULL) + ((v >> 2) & 0x3333333333333333ULL);
        v = (v + (v >> 4)) & 0xf0f0f0f0f0f0f0fULL;
        return (v * 0x101010101010101ULL) >> 56;
    }

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & docid & length & fp & attach;
    }

public:
    DocIdType docid;
    uint32_t length;
    int status; //1 means its new, 0 means old, not serialized.
    FpType fp;
    AttachType attach;
};

NS_IDMLIB_DD_END

#endif
