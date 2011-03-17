#ifndef IDMLIB_SIM_SIMPLESIMILARITY_H_
#define IDMLIB_SIM_SIMPLESIMILARITY_H_


#include <string>
#include <iostream>
#include <idmlib/idm_types.h>
#include <am/matrix/sparse_vector.h>
NS_IDMLIB_SIM_BEGIN


template <class T>
class SimpleSimilarity
{
public:
  
  static double Sim(const T& vec1, const T& vec2)
  {
    if(vec1.size()!=vec2.size()) return 0.0;
    uint64_t dist = 0;

    for(typename T::size_type i=0;i<vec1.size();i++)
    {
      int64_t d = vec1[i]-vec2[i];
      if(d<0)
      {
        d*=-1;
      }
      dist += (uint64_t)d;
    }
    double r = 100.0;
    if(dist>0)
    {
      r = 1.0/dist;
    }
    return r;
  }
  
};

// template <>
// class CosineSimilarity<izenelib::am::SparseVector>
// {
// public:
//   
//   static double Sim(const izenelib::am::SparseVector& vec1, const izenelib::am::SparseVector& vec2)
//   {
//     //TODO
//     return 0.0;
//   }
//   
// };

   
NS_IDMLIB_SIM_END



#endif 
