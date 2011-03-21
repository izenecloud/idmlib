#ifndef IDMLIB_SIM_COSINESIMILARITY_H_
#define IDMLIB_SIM_COSINESIMILARITY_H_


#include <string>
#include <iostream>
#include <idmlib/idm_types.h>
#include <am/matrix/sparse_vector.h>
NS_IDMLIB_SIM_BEGIN


class CosineSimilarity
{
public:
  
  template <typename T>
  static double Sim(const std::vector<T>& vec1, const std::vector<T>& vec2)
  {
    if(vec1.size()!=vec2.size()) return 0.0;
    int64_t p = 0;
    uint64_t n1 = 0;
    uint64_t n2 = 0;
    for(typename std::vector<T>::size_type i=0;i<vec1.size();i++)
    {
      p += vec1[i]*vec2[i];
      n1 += vec1[i]*vec1[i];
      n2 += vec2[i]*vec2[i];
    }
    double d1 = std::sqrt((double)n1);
    double d2 = std::sqrt((double)n2);
    return (double)p/d1/d2;
  }
  
  template <typename V, typename K>
  static double Sim(const izenelib::am::SparseVector<V,K>& vec1, const izenelib::am::SparseVector<V,K>& vec2)
  {
//     std::cout<<"[cosine] "<<vec1.value.size()<<","<<vec2.value.size()<<std::endl;
    V p = 0;
    V n1 = 0;
    V n2 = 0;
    
    K p1 = 0;
    K p2 = 0;
    while(p1<vec1.value.size() && p2<vec2.value.size() )
    {
      if(vec1.value[p1].first<vec2.value[p2].first)
      {
        n1 += vec1.value[p1].second * vec1.value[p1].second;
        p1++;
      }
      else if(vec1.value[p1].first>vec2.value[p2].first)
      {
        n2 += vec2.value[p2].second * vec2.value[p2].second;
        p2++;
      }
      else
      {
        p += vec1.value[p1].second * vec2.value[p2].second;
        n1 += vec1.value[p1].second * vec1.value[p1].second;
        n2 += vec2.value[p2].second * vec2.value[p2].second;
        p1++;
        p2++;
      }
    }
    while(p1<vec1.value.size())
    {
      n1 += vec1.value[p1].second * vec1.value[p1].second;
      p1++;
    }
    while(p2<vec2.value.size())
    {
      n2 += vec2.value[p2].second * vec2.value[p2].second;
      p2++;
    }
    double result = -100.0;
    if(n1>0 && n2>0)
    {
      double d1 = std::sqrt((double)n1);
      double d2 = std::sqrt((double)n2);
      result = (double)p/d1/d2;
    }
//     std::cout<<result<<std::endl;
    return result;
  }
  
};

// template <>
// class CosineSimilarity<izenelib::am::SparseVector<>
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
