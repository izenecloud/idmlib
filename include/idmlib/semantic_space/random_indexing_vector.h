#ifndef IDMLIB_SSP_RANDOMINDEXINGVECTOR_H_
#define IDMLIB_SSP_RANDOMINDEXINGVECTOR_H_


#include <string>
#include <iostream>
#include <idmlib/idm_types.h>
#include <am/matrix/sparse_vector.h>
#include <boost/serialization/access.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
NS_IDMLIB_SSP_BEGIN

template <typename T = uint16_t>
class RandomIndexingVector
{
public:

  template <typename U, typename I>
  void AddTo(std::vector<U>& value_vector, I freq = 1)
  {
    for(uint32_t i=0;i<positive_position.size();i++)
    {
      value_vector[positive_position[i]] += freq;
    }
    for(uint32_t i=0;i<negative_position.size();i++)
    {
      value_vector[negative_position[i]] -= freq;
    }
  }
  
//   template <typename V, typename K>
//   void AddTo(izenelib::am::SparseVector<V,K>& value_vector)
//   {
//     //TODO
//   }

  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
      ar & positive_position & negative_position;
  }
  
 
public: 
  std::vector<T> positive_position;
  std::vector<T> negative_position;
};

   
NS_IDMLIB_SSP_END



#endif 
