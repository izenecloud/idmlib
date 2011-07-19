#ifndef IDMLIB_SSP_APSSPERSIST_H_
#define IDMLIB_SSP_APSSPERSIST_H_


#include <string>
#include <iostream>
#include <vector>
#include <boost/function.hpp>
#include <idmlib/idm_types.h>
#include <am/matrix/sparse_vector.h>
#include <am/sequence_file/ssfr.h>
#include <list>
NS_IDMLIB_SSP_BEGIN

template <typename IdType, class SVType>
class ApssPersist
{
  public:
    
    ApssPersist(const std::string& dir)
    :dir_(dir)
    {
    }
    
    bool Load(std::vector<SVType>& inverted_index, IdType& max_id)
    {
      std::string inverted_file = dir_+"/inverted";
      std::string max_file = dir_+"/max_id";
      if(!izenelib::am::ssf::Util<uint32_t>::Load(inverted_file, inverted_index)) return false;
      if(!izenelib::am::ssf::Util<uint32_t>::Load(max_file, max_id)) return false;
      return true;
    }
    
    bool Load(IdType& max_id)
    {
      std::string max_file = dir_+"/max_id";
      if(!izenelib::am::ssf::Util<uint32_t>::Load(max_file, max_id)) return false;
      return true;
    }
    
    bool Save(const std::vector<SVType>& inverted_index, IdType max_id)
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
      std::string inverted_file = dir_+"/inverted";
      std::string max_file = dir_+"/max_id";
      if(!izenelib::am::ssf::Util<uint32_t>::Save(inverted_file, inverted_index)) return false;
      if(!izenelib::am::ssf::Util<uint32_t>::Save(max_file, max_id)) return false;
      return true;
    }
    
    bool Save(IdType max_id)
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
      std::string max_file = dir_+"/max_id";
      if(!izenelib::am::ssf::Util<uint32_t>::Save(max_file, max_id)) return false;
      return true;
    }
    
  private:

  
  private:
    std::string dir_;
};

   
NS_IDMLIB_SSP_END



#endif 
