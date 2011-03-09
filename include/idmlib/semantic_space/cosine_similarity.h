#ifndef IDMLIB_SSP_COSINESIMILARITY_H_
#define IDMLIB_SSP_COSINESIMILARITY_H_


#include <string>
#include <iostream>
#include <idmlib/idm_types.h>
#include "sparse_vector.h"
NS_IDMLIB_SSP_BEGIN


template <class T>
class CosineSimilarity
{
public:
  
  static double Sim(const T& vec1, const T& vec2)
  {
    if(vec1.size()!=vec2.size()) return 0.0;
    int64_t p = 0;
    uint64_t n1 = 0;
    uint64_t n2 = 0;
    for(T::size_t i=0;i<vec1.size();i++)
    {
      p += vec1[i]*vec2[i];
      n1 += vec1[i]*vec1[i];
      n2 += vec2[i]*vec2[i];
    }
    double d1 = std::sqrt((double)n1);
    double d2 = std::sqrt((double)n2);
    return (double)p/d1/d2;
  }
  
};

template <>
class CosineSimilarity<SparseVector>
{
public:
  
  static double Sim(const SparseVector& vec1, const SparseVector& vec2)
  {
    //TODO
    return 0.0;
  }
  
};

   
NS_IDMLIB_SSP_END



#endif 
