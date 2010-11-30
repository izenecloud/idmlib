///
/// @file Util.hpp
/// @brief Utility for iDMlib
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2010-04-09
/// @date 
///

#ifndef IDM_RESOURCEUTIL_H_
#define IDM_RESOURCEUTIL_H_

#include <string>

#include <util/ProcMemInfo.h>
#include <util/ustring/UString.h>
#include <util/ustring/algo.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/crc.hpp>  // for boost::crc_32_type
#include "FSUtil.hpp"
#include <idmlib/idm_types.h>
#include <util/bzip.h>

NS_IDMLIB_UTIL_BEGIN

inline static uint32_t getCRCValue(char* data, int len)
{
  boost::crc_32_type  result;
  result.process_bytes(data, len);
  uint32_t crc_value = result.checksum();
  return crc_value;
}

inline static std::istream* decompressStream(const std::string& res_name, const std::string& crc_name)
{
  std::ifstream crc_file(crc_name.c_str());
  if(!crc_file)
  {
    std::cerr<<"open crc file "<<crc_name<<" error"<<std::endl;
    return NULL;
  }
  uint32_t crc_value = 0;
  crc_file>>crc_value;
  if(crc_file.fail())
  {
    std::cerr<<"read crc file "<<crc_name<<" error"<<std::endl;
    return NULL;
  }
  crc_file.close();
  std::ifstream file(res_name.c_str());
  if(!file)
  {
    std::cerr<<"open resource file "<<res_name<<" error"<<std::endl;
    return NULL;
  }
  file.seekg(0, std::ios_base::end);
  int len = file.tellg();
  file.seekg(0, std::ios_base::beg);
  char* data = (char*)malloc(len);
  file.read(data, len);
  file.close();
  uint32_t c_crc_value = getCRCValue(data, len);
  if( c_crc_value != crc_value )
  {
    std::cerr<<"crc check failed in "<<crc_name<<", plz check the resource file."<<std::endl;
    free(data);
    return NULL;
  }
  int decompressLen = 0;
  char* decompressData = izenelib::util::_tc_bzdecompress(data, len, &decompressLen);
  free(data);
  
  std::stringstream* result = new std::stringstream();
  result->write( decompressData, decompressLen);
  free(decompressData);
  result->seekg(0, std::ios_base::beg);
  return result;
}

inline static std::istream* getResourceStream(const std::string& name)
{
    using namespace boost::iostreams;
    std::string res_name = name+".res";
    std::string crc_name = name+".crc";
    try
    {
      if( boost::filesystem::exists(res_name) && boost::filesystem::exists(crc_name) )
      {
        return decompressStream(res_name,crc_name);
      }
      else
      {
        std::cerr<<"can not find resource data for "<<name<<std::endl;
        return NULL;
      }
    }
    catch(std::exception& ex)
    {
      std::cerr<<"some error happen when loading resource "<<name<<std::endl;
      return NULL;
    }
}

inline static bool generateResource(const std::string& name)
{
  std::string res_name = name+".res";
  std::string crc_name = name+".crc";
  std::ifstream file(name.c_str());
  if(!file)
  {
    std::cerr<<"open "<<name<<" failed"<<std::endl;
    return false;
  }
  file.seekg(0, std::ios_base::end);
  int len = file.tellg();
  file.seekg(0, std::ios_base::beg);
  char* data = (char*)malloc(len);
  file.read(data, len);
  file.close();
  int compressLen = 0;
  char* compressData = izenelib::util::_tc_bzcompress(data, len, &compressLen);
  free(data);
  std::ofstream res_output(res_name.c_str());
  res_output.write(compressData, compressLen);
  res_output.close();
  uint32_t crc_value = getCRCValue(compressData, compressLen);
  std::ofstream crc_output(crc_name.c_str());
  crc_output<<crc_value;
  crc_output.close();
  return true;
}




NS_IDMLIB_UTIL_END

#endif
