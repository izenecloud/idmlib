#ifndef IDMLIB_SSP_RANDOMINDEXINGGENERATOR_H_
#define IDMLIB_SSP_RANDOMINDEXINGGENERATOR_H_


#include <string>
#include <iostream>
#include <idmlib/idm_types.h>
#include <boost/random/uniform_int.hpp>
#include <boost/random.hpp>
#include <boost/lexical_cast.hpp>
#include <am/matrix/matrix_mem_io.h>
#include "random_indexing_vector.h"
#include <am/sequence_file/ssfr.h>
NS_IDMLIB_SSP_BEGIN


///@brief Used to generate RandomIndexingVector in specific dimensions and random probability.
template <typename T, typename I = uint32_t>
class RandomIndexingGenerator 
{
public:
  typedef RandomIndexingVector<T> RIVType;

  ///@brief Set the dimensions while the probability for generating 1 or -1 equals p/dimensions.
  RandomIndexingGenerator(const std::string& dir, T dimensions, T p)
  :dir_(dir), dimensions_(dimensions), p_(p)
  ,engine_(), distribution_(0, dimensions_), random_(engine_, distribution_)
  {
    ser_file_ = dir_+"/"+boost::lexical_cast<std::string>(dimensions_)+"-"+boost::lexical_cast<std::string>(p_);
  }
  
  bool Open()
  {
    try
    {
      boost::filesystem::create_directories(dir_);
    }
    catch(std::exception& ex)
    {
      std::cerr<<ex.what()<<std::endl;
      return false;
    }
    return true;
  }
  
  void Clear()
  {
    value_.clear();
  }
  
  const RIVType& Get(I id)
  {
    return value_[id-1];
  }
  
  bool ResetMax(I max_id)
  {
    std::cout<<"[RIG] ResetMax : "<<max_id<<std::endl;
    value_.clear();
    value_.resize(max_id);
    I start = 1;
    {
      izenelib::am::ssf::Reader<uint32_t> reader(ser_file_);
      
      if(reader.Open())
      {
        RIVType riv;
        while(reader.Next(riv))
        {
//           std::cout<<"load "<<start<<std::endl;
          value_[start-1] = riv;
          start++;
          if(start>max_id) break;
        }
      }
      reader.Close();
    }
    for(I i=start;i<=max_id;i++)
    {
      if(i%1000==0)
      {
        std::cout<<"rig processing "<<i<<std::endl;
      }
      for(T t=0;t<dimensions_;t++)
      {
        T random = random_();
        if(random<p_)
        {
          value_[i-1].positive_position.push_back(t);
        }
        else if(random<p_*2)
        {
          value_[i-1].negative_position.push_back(t);
        }
      }
    }
    if(!Flush_())
    {
      std::cerr<<"rig Flush error"<<std::endl;
      return false;
    }
    return true;
  }

  
  
  inline T GetDimensions() const
  {
    return dimensions_;
  }
  
 private:
  bool Flush_()
  {
    boost::filesystem::remove_all(ser_file_);
    izenelib::am::ssf::Writer<uint32_t> writer(ser_file_);
    if(!writer.Open() )
    {
      return false;
    }
    for(I i=0;i<value_.size();i++)
    {
      if(!writer.Append(value_[i]))
      {
        return false;
      }
    }
    writer.Close();
    return true;
    
  }
  
 private: 
  std::string dir_;
  T dimensions_;
  T p_;
  std::vector<RIVType> value_;
  std::string ser_file_;
  boost::mt19937 engine_;
  boost::uniform_int<T> distribution_;
  boost::variate_generator<boost::mt19937, boost::uniform_int<T> > random_;
};

// typedef RandomIndexingGenerator<uint16_t, uint32_t, MatrixMemIo> CommonRIGType;

   
NS_IDMLIB_SSP_END



#endif 
