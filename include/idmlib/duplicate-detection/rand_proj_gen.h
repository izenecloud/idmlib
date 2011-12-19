/**
   @file rand_proj_gen.h
   @author Kevin Hu
   @date   2009.11.24
 */

#ifndef IDMLIB_DD_RAND_PROJ_GEN_H
#define IDMLIB_DD_RAND_PROJ_GEN_H

#include <idmlib/idm_types.h>
#include "rand_proj.h"

#include "boost/random/linear_congruential.hpp"
#include "boost/random/uniform_real.hpp"
#include "boost/random/variate_generator.hpp"
#include <am/graph_index/dyn_array.hpp>
#include <string>

NS_IDMLIB_DD_BEGIN

/**
   @brief token and projection pair
 */
struct UNIT_STRUCT
{
    uint32_t token;
    uint64_t proj;

    inline UNIT_STRUCT(uint32_t i, uint64_t j)
    {
        token = i;
        proj = j;
    }

    inline UNIT_STRUCT(uint32_t i)
    {
        token = i;
        proj = -1;
    }

    inline UNIT_STRUCT()
    {
        token = 0;
        proj = -1;
    }

    inline UNIT_STRUCT(const UNIT_STRUCT& other)
    {
        token = other.token;
        proj = other.proj;
    }

    inline UNIT_STRUCT& operator = (const UNIT_STRUCT& other)
    {
        token = other.token;
        proj = other.proj;
        return *this;
    }

    inline bool operator == (const UNIT_STRUCT& other)const
    {
        return (token == other.token);
    }

    inline bool operator != (const UNIT_STRUCT& other)const
    {
        return (token != other.token);
    }

    inline bool operator < (const UNIT_STRUCT& other)const
    {
        return (token < other.token);
    }

    inline bool operator > (const UNIT_STRUCT& other)const
    {
        return (token > other.token);
    }

    inline bool operator <= (const UNIT_STRUCT& other)const
    {
        return (token <= other.token);
    }

    inline bool operator >= (const UNIT_STRUCT& other)const
    {
        return (token >= other.token);
    }

    inline uint32_t operator % (uint32_t e)const
    {
        return (token % e);
    }

    friend std::ostream& operator << (std::ostream& os, const UNIT_STRUCT& v)
    {
        os<<"<"<<v.token<<","<<v.proj<<">";
        return os;
    }
}
;

/**
   @class RandProjGen
 * @brief Engine for Random Projection class.
 */
class RandProjGen
{
private:
    static boost::minstd_rand rangen;//!< it's useless now
    static boost::uniform_real<> uni_real;//!< it's useless now
    static boost::variate_generator<boost::minstd_rand&, boost::uniform_real<> > U;//!< it's useless now

    typedef izenelib::am::DynArray<struct UNIT_STRUCT> slot_t;
    typedef izenelib::am::DynArray<slot_t*>  entry_t;

    const uint32_t ENTRY_SIZE_;//!< it's useless now
    int nDimensions_; //!< dimension size
    entry_t entry_;//!<it's useless now
    std::string nm_;//!< it's useless now

    /**
       @brief it's useless now
     */
    RandProj* map_find_(uint32_t token);

    /**
       @brief it's useless now
     */
    void map_insert_(uint32_t token, RandProj* proj);

    /**
       @brief it's useless now
     */
    void map_save_();

    /**
       @brief it's useless now
     */
    void map_load_();

public:
    /**
       @brief a constructor
       @param filenm file name to store map.
       @param n dimension size.
     */
    explicit inline RandProjGen(const char* filenm, int n)
            : ENTRY_SIZE_(1000000), nDimensions_(n), nm_(filenm)
    {
        map_load_();
    }

    /**
       @brief a destructor
     */
    inline ~RandProjGen()
    {
        map_save_();
    }

    /**
       @brief get dimension size
       @return a int indicates dimension size
     */
    inline int num_dimensions() const
    {
        return nDimensions_;
    }

    /**
       @brief flush data onto disk without deallocation
     */
    void commit();

    /**
      @brief method to get a random projection.
    */
    RandProj get_random_projection(uint32_t token);

    /**
      @brief method to get a random projection.
    */
    RandProj get_random_projection(const std::string& token);
};
NS_IDMLIB_DD_END

#endif
