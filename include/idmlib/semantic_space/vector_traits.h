#ifndef IDMLIB_SSP_VECTORTRAITS_H_
#define IDMLIB_SSP_VECTORTRAITS_H_


#include <string>
#include <iostream>
#include <vector>
#include <idmlib/idm_types.h>
#include <am/matrix/sparse_vector.h>

NS_IDMLIB_SSP_BEGIN

template <class VT>
class VectorTraits
{
  public:
    typedef std::vector<double> NormalizedType;
};

template <typename V, typename K>
class VectorTraits<izenelib::am::SparseVector<V,K> >
{
  public:
    typedef izenelib::am::SparseVector<double,K> NormalizedType;
};

// template <class V, typename K, class VectorType>
// class VectorTraits
// {
// public:
//   static void Resize(VectorType& v, typename VectorType::size_type size, const typename VectorType::value_type& value)
//   {
//     v.resize(size, value);
//   }
// };
// 
// template <>
// class VectorTraits<izenelib::am::SparseVector<uint32_t, uint32_t> >
// {
// public:
//   static void Resize(VectorType& v, typename VectorType::size_type size, const typename VectorType::value_type& value)
//   {
//     v.resize(size, value);
//   }
// };

   
NS_IDMLIB_SSP_END



#endif 
