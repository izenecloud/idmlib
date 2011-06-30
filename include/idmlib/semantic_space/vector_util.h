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
    
    template<typename V, typename T, typename I>
    inline static void VectorAdd(izenelib::am::SparseVector<V, T>& to, const izenelib::am::SparseVector<V, T>& from, I times)
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
        new_value[p].first = from.value[i].first;
        new_value[p].second = from.value[i].second*times;
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
    
    template<typename V, typename T>
    inline static void VectorAdd(izenelib::am::SparseVector<V, T>& to, const izenelib::am::SparseVector<V, T>& from)
    {
      VectorAdd<V,T, uint32_t>(to, from, 1);
    }
    
    
    template<typename V, typename T, typename I>
    inline static void Push(izenelib::am::SparseVector<V, T>& to, const izenelib::am::SparseVector<V, T>& from, I times)
    {
      T p = to.value.size();
      to.value.resize(to.value.size()+from.value.size());
      
      for(uint32_t i=0;i<from.value.size();i++)
      {
        to.value[p].first = from.value[i].first;
        to.value[p].second = from.value[i].second*times;
        ++p;
      }       
    }
    
    template<typename V, typename T>
    inline static void Flush(izenelib::am::SparseVector<V, T>& vec)
    {
      std::vector<std::pair<T, V> > new_value(vec.value.begin(), vec.value.end());
      std::sort(new_value.begin(), new_value.end());
      T key = 0;
      V value = 0;
      vec.value.resize(0);
      for(uint32_t i=0;i<new_value.size();i++)
      {
        T current_key = new_value[i].first;
        if(current_key!=key && value!=0)
        {
          vec.value.push_back(std::make_pair(key, value) );
          value = 0;
        }
        value += new_value[i].second;
        key = current_key;
      }
      if(value!=0)
      {
        vec.value.push_back(std::make_pair(key, value) );
      }      
    }
    
    template<typename V, typename T>
    inline static void VectorMultiple(const izenelib::am::SparseVector<V, T>& from, izenelib::am::SparseVector<V, T>& to, int m)
    {
        to.value.assign(from.value.begin(), from.value.end());
        for(uint32_t i=0; i<to.value.size();i++)
        {
            to.value[i].second *= m;
        }
    }
    
    template<typename V, typename D, typename T>
    inline static void VectorCopy(const izenelib::am::SparseVector<V, T>& from, izenelib::am::SparseVector<D, T>& to)
    {
        to.value.resize(from.value.size());
        for(uint32_t i=0; i<to.value.size();i++)
        {
            to.value[i].first = from.value[i].first;
            to.value[i].second = (D)from.value[i].second;
        }
    }
};

   
NS_IDMLIB_SSP_END



#endif 
