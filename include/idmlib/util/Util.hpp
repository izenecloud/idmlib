///
/// @file Util.hpp
/// @brief Utility for iDMlib
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2010-04-09
/// @date 
///

#ifndef IDM_UTIL_HPP_
#define IDM_UTIL_HPP_

#include <string>

#include <util/ProcMemInfo.h>
#include <util/ustring/UString.h>
#include <util/ustring/algo.hpp>
#include "FSUtil.hpp"
#include <idmlib/idm_types.h>

NS_IDMLIB_UTIL_BEGIN

typedef std::pair<uint32_t, uint32_t> id2count_t;
            
inline static uint64_t make64UInt(uint32_t int1, uint32_t int2)
{
    uint64_t r = int1;
    r = r<<32;
    r += int2;
    return r;
}

inline static void VMINFO(const std::string& str = "")
{
    unsigned long rlimit;
    static unsigned long  vm = 0, rss = 0;
    unsigned long pre_vm = vm;
    
    
    sleep(2);
    izenelib::util::ProcMemInfo::getProcMemInfo(vm, rss, rlimit);
    std::cout<<"["<< str<<"] : " << vm << " bytes; \t" ;
    if(vm >= pre_vm )
        std::cout << "++++ : " << vm - pre_vm << " bytes;";               
    else 
        std::cout << "---- : " <<  pre_vm - vm << " bytes;";
    std::cout<<std::endl;
}

inline static void RSSINFO(const std::string& str = "")
{
    unsigned long rlimit;
    static unsigned long  vm = 0, rss = 0;
    unsigned long pre_rss = rss;
    
    sleep(2);
    izenelib::util::ProcMemInfo::getProcMemInfo(vm, rss, rlimit);
    std::cout<<"["<< str<<"] : " << rss << " bytes; \t" ;
    if( rss >= pre_rss )
        std::cout <<"++++ : " << rss - pre_rss  << " bytes.";
    else
        std::cout <<"---- : " << pre_rss - rss  << " bytes.";
    std::cout<<std::endl;
}

inline static void MEMINFO(const std::string& str = "")
{
    RSSINFO(str);
}

inline static void print( const izenelib::util::UString& ustr)
{
    std::string str;
    ustr.convertString(str, izenelib::util::UString::UTF_8);
    std::cout<<str;
}



inline static void getIdAndCountList(std::vector<id2count_t>& input, std::vector<uint32_t>& idList, std::vector<uint32_t>& countList)
{
    std::sort(input.begin(), input.end());
    for(uint32_t i=0;i<input.size();i++)
    {
        if(i==0)
        {
            idList.push_back(input[i].first);
            countList.push_back(input[i].second);
        }
        else
        {
            if( idList.back() != input[i].first )
            {
                idList.push_back(input[i].first);
                countList.push_back(input[i].second);
            }
            else
            {
                countList[countList.size()-1] += input[i].second;
            }
        }
    }
}

template <class T>
inline static void accumulateList(std::vector<std::pair<T, uint32_t> >& input)
{
  std::sort(input.begin(), input.end());
  std::vector<std::pair<T, uint32_t> > another;
  for(uint32_t i=0;i<input.size();i++)
  {
      if(i==0)
      {
          another.push_back(input[i]);
      }
      else
      {
          if( another.back().first != input[i].first )
          {
              another.push_back(input[i]);
          }
          else
          {
              another.back().second += input[i].second;
          }
      }
  }
  input.resize(0);
  input.insert(input.end(), another.begin(), another.end() );
}

NS_IDMLIB_UTIL_END

#endif
