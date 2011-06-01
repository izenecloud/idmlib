#ifndef IDMLIB_SSP_RANDOMINDEXINGVECTORTRAITS_H_
#define IDMLIB_SSP_RANDOMINDEXINGVECTORTRAITS_H_


#include <string>
#include <iostream>
#include <vector>
#include <idmlib/idm_types.h>
#include <am/sequence_file/ssfr.h>
#include "vector_util.h"
NS_IDMLIB_SSP_BEGIN

template <typename T>
class RandomIndexingVectorTraits
{
    public:
        typedef int64_t ValueType;
        static ValueType GetZero()
        {
            return 0;
        }
};

template <>
class RandomIndexingVectorTraits<double>
{
    public:
        typedef double ValueType;
        
        static ValueType GetZero()
        {
            return 0.0;
        }
};

   
NS_IDMLIB_SSP_END



#endif 
