#ifndef IDMLIB_SSP_NORMALIZER_H_
#define IDMLIB_SSP_NORMALIZER_H_


#include <string>
#include <iostream>
#include <vector>
#include <idmlib/idm_types.h>
#include <am/matrix/sparse_vector.h>

NS_IDMLIB_SSP_BEGIN

class Normalizer
{
public:
  template <typename T>
  static void Normalize(const std::vector<T>& vec, std::vector<double>& result)
  {
    T sum = 0;
    for(uint32_t i=0;i<vec.size();i++)
    {
      if(vec[i]>0)
      {
        sum += vec[i]*vec[i];
      }
    }
    result.resize(vec.size(), 0.0);
    if(sum>0)
    {
      double c = std::sqrt((double)sum);
      for(uint32_t i=0;i<vec.size();i++)
      {
        result[i] = (double)vec[i]/c;
      }
    }
  }
  
  template <typename V, typename K>
  static void Normalize(const std::vector<V>& vec, izenelib::am::SparseVector<double,K>& result)
  {
    V sum = 0;
    for(K i=0;i<vec.size();i++)
    {
      sum += vec[i]*vec[i];
    }
    if(sum>0)
    {
      double c = std::sqrt((double)sum);
      for(K i=0;i<vec.size();i++)
      {
        if(vec[i]>0)
        {
          double r = (double)vec[i]/c;
          result.value.push_back(std::make_pair(i, r));
        }
      }
    }
  }
  
  template <typename V, typename K>
  static void Normalize(const izenelib::am::SparseVector<V,K>& vec, izenelib::am::SparseVector<double,K>& result)
  {
    V sum = 0;
    for(K i=0;i<vec.value.size();i++)
    {
      sum += vec.value[i].second*vec.value[i].second;
    }
    result.value.resize(vec.value.size());
    for(K i=0;i<vec.value.size();i++)
    {
      result.value[i].first = vec.value[i].first;
      result.value[i].second = 0.0;
    }
    if(sum>0)
    {
      double c = std::sqrt((double)sum);
      for(K i=0;i<vec.value.size();i++)
      {
        result.value[i].second = (double)vec.value[i].second/c;
      }
    }
    
//     {
//       for(uint32_t i=0;i<result.value.size();i++)
//       {
//         std::cout<<result.value[i].first<<","<<result.value[i].second<<" ";
//       }
//       std::cout<<std::endl;
//     }
  }
};

   
NS_IDMLIB_SSP_END



#endif 
