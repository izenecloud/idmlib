#ifndef IDMLIB_SSP_RANDOMINDEXINGVECTOR_H_
#define IDMLIB_SSP_RANDOMINDEXINGVECTOR_H_


#include <string>
#include <iostream>
#include <idmlib/idm_types.h>
#include "sparse_vector.h"
NS_IDMLIB_SSP_BEGIN

template <typename T = uint16_t>
class RandomIndexingVector
{
public:

  template <typename U>
  void AddTo(std::vector<U>& value_vector)
  {
    for(uint32_t i=0;i<positive_position.size();i++)
    {
      ++value_vector[positive_position[i]];
    }
    for(uint32_t i=0;i<negative_position.size();i++)
    {
      --value_vector[negative_position[i]];
    }
  }
  
  template <typename V, typename K>
  void AddTo(SparseVector& value_vector)
  {
    //TODO
  }

  
 
public: 
  std::vector<T> positive_position;
  std::vector<T> negative_position;
};

   
NS_IDMLIB_SSP_END



#endif 
