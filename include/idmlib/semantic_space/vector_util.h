#ifndef IDMLIB_SSP_VECTORUTIL_H_
#define IDMLIB_SSP_VECTORUTIL_H_


#include <string>
#include <iostream>
#include <vector>
#include <idmlib/idm_types.h>
#include <am/matrix/sparse_vector.h>

NS_IDMLIB_SSP_BEGIN


class VectorUtil
{
  public:
    
    template<typename T, typename K>
    inline static void ConvertToSparse(const std::vector<T>& vec, izenelib::am::SparseVector<T, K>& s_vec)
    {
      for(K i=0;i<vec.size();i++)
      {
        if(vec[i]>0)
        {
          s_vec.value.push_back(std::make_pair(i, vec[i]));
        }
      }
    }
    
    template<typename V, typename T>
    inline static void VectorAdd(izenelib::am::SparseVector<V, T>& to, const izenelib::am::SparseVector<V, T>& from)
    {
      std::vector<std::pair<T, V> > new_value(to.value.size()+from.value.size());
      T p = 0;
      for(uint32_t i=0;i<to.value.size();i++)
      {
        new_value[p] = to.value[i];
        ++p;
      }
      for(uint32_t i=0;i<from.value.size();i++)
      {
        new_value[p] = from.value[i];
        ++p;
      }

      std::sort(new_value.begin(), new_value.end());
      T key = 0;
      V value = 0;
      to.value.resize(0);
      for(uint32_t i=0;i<new_value.size();i++)
      {
        T current_key = new_value[i].first;
        if(current_key!=key && value!=0)
        {
          to.value.push_back(std::make_pair(key, value) );
          value = 0;
        }
        value += new_value[i].second;
        key = current_key;
      }
      if(value!=0)
      {
        to.value.push_back(std::make_pair(key, value) );
      }
    }
};

   
NS_IDMLIB_SSP_END



#endif 
