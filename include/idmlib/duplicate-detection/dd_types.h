#ifndef IDMLIB_DD_DDTYPES_H_
#define IDMLIB_DD_DDTYPES_H_

#include "dd_constants.h"
#include <boost/serialization/access.hpp>
#include <boost/serialization/serialization.hpp>


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

struct SimHash
{
    static const uint32_t FP_SIZE = DdConstants::f / 64;

    uint64_t desc[FP_SIZE];

    SimHash() : desc() {}

    SimHash(const SimHash& other)
    {
        operator=(other);
    }

    int compare(const SimHash& other) const
    {
        for (unsigned i = 0; i < FP_SIZE; i++)
        {
            if (desc[i] < other.desc[i])
                return -1;
            else if (desc[i] > other.desc[i])
                return 1;
        }
        return 0;
    }

    bool operator<(const SimHash& other) const
    {
        return compare(other) == -1;
    }

    bool operator!=(const SimHash& other) const
    {
        return compare(other) != 0;
    }

    const SimHash& operator=(const SimHash& other)
    {
        for (unsigned i = 0; i < FP_SIZE; i++)
        {
            desc[i] = other.desc[i];
        }
        return *this;
    }

private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        for (unsigned i = 0; i < FP_SIZE; i++)
            ar & desc[i];
    }

};

NS_IDMLIB_DD_END

#endif /* DUPDETECTOR_H_ */
