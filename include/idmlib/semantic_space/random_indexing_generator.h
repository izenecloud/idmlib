#ifndef IDMLIB_SSP_RANDOMINDEXINGGENERATOR_H_
#define IDMLIB_SSP_RANDOMINDEXINGGENERATOR_H_


#include <string>
#include <iostream>
#include <idmlib/idm_types.h>
#include <boost/random/uniform_int.hpp>
#include <boost/random.hpp>
#include <am/matrix/matrix_mem_io.h>
#include "random_indexing_vector.h"
NS_IDMLIB_SSP_BEGIN


///@brief Used to generate RandomIndexingVector in specific dimensions and random probability.
template <typename T, typename I = uint32_t, template <typename VT, typename TI> class MatrixIo = izenelib::am::MatrixMemIo >
class RandomIndexingGenerator : public MatrixIo<RandomIndexingVector<T> , I>
{
public:
  typedef RandomIndexingVector<T> RIVType;
  typedef MatrixIo<RIVType, I> MatrixIoType;
  
  ///@brief Set the dimensions while the probability for generating 1 or -1 equals p/dimensions.
  RandomIndexingGenerator(const std::string& dir, T dimensions, T p)
  :MatrixIoType(dir), dir_(dir), dimensions_(dimensions), p_(p)
  ,engine_(), distribution_(0, dimensions_), random_(engine_, distribution_)
  {
  }
  
  ///@brief Generate the RandomIndexingVector for id if not exists.
  bool Get(I id, RIVType& item)
  {
    if(GetVector(id, item))
    {
      return true;
    }
    else
    {
      
      for(T t=0;t<dimensions_;t++)
      {
        T random = random_();
        if(random<p_)
        {
          item.positive_position.push_back(t);
        }
        else if(random<p_*2)
        {
          item.negative_position.push_back(t);
        }
      }
      if(!SetVector(id, item))
      {
        std::cerr<<"set random vector for id "<<id<<" failed."<<std::endl;
        return false;
      }
      return true;
    }
    
  }
  
  T GetDimensions() const
  {
    return dimensions_;
  }
  
  
 private: 
  std::string dir_;
  T dimensions_;
  T p_;
  boost::mt19937 engine_;
  boost::uniform_int<T> distribution_;
  boost::variate_generator<boost::mt19937, boost::uniform_int<T> > random_;
};

// typedef RandomIndexingGenerator<uint16_t, uint32_t, MatrixMemIo> CommonRIGType;

   
NS_IDMLIB_SSP_END



#endif 
