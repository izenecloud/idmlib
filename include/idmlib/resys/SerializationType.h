#ifndef IDMLIB_RESYS_SERIALIZATIONTYPES_H
#define IDMLIB_RESYS_SERIALIZATIONTYPES_H

#include <idmlib/idm_types.h>

#include <util/timestamp.h>
#include <util/izene_serialization.h>

#include <3rdparty/am/google/sparse_hash_map>
#include <ext/hash_map>
#include <boost/unordered_map.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/hash_map.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

NS_IDMLIB_RESYS_BEGIN

struct CoVisitTimeFreq
{
    CoVisitTimeFreq():freq(0),timestamp(0){}

    uint32_t freq;
    int64_t timestamp;

    void update()
    {
        freq += 1;
        timestamp = (int64_t)izenelib::util::Timestamp::now();
    }

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar &freq & timestamp;
    }    
};

struct CoVisitFreq
{
    CoVisitFreq():freq(0){}

    uint32_t freq;

    void update()
    {
        freq += 1;
    }

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar &freq;
    }
};


NS_IDMLIB_RESYS_END

typedef google::sparse_hash_map<uint32_t, float> UINT_FLOAT_HASH_TYPE;
typedef google::sparse_hash_map<uint32_t, uint8_t> UINT_BYTE_HASH_TYPE;
typedef google::sparse_hash_map<uint32_t, idmlib::recommender::CoVisitFreq> UINT_COVISITFREQ_HASH_TYPE;
typedef google::sparse_hash_map<uint32_t, idmlib::recommender::CoVisitTimeFreq> UINT_COVISITTIMEFREQ_HASH_TYPE;

MAKE_FEBIRD_SERIALIZATION(UINT_FLOAT_HASH_TYPE)
MAKE_FEBIRD_SERIALIZATION(UINT_BYTE_HASH_TYPE)
MAKE_FEBIRD_SERIALIZATION(UINT_COVISITFREQ_HASH_TYPE)
MAKE_FEBIRD_SERIALIZATION(UINT_COVISITTIMEFREQ_HASH_TYPE)


#endif
