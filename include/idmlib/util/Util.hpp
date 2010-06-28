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
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include "FSUtil.hpp"
#include <idmlib/idm_types.h>
#include <util/bzip.h>

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

inline static std::istream* decompressStream(const std::string& fileName)
{
    std::ifstream file(fileName.c_str());
    file.seekg(0, std::ios_base::end);
    int len = file.tellg();
    file.seekg(0, std::ios_base::beg);
    char* data = (char*)malloc(len);
    file.read(data, len);
    file.close();
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
    std::string resName = name+".res";
    std::string txtName = name+".txt";
    if( FSUtil::exists(resName) && FSUtil::exists(txtName) )
    {
        if( boost::filesystem::last_write_time(txtName) >
            boost::filesystem::last_write_time(resName) )
        {
            FSUtil::del(resName);
        }
    }
    if( FSUtil::exists(resName) )
    {
        return decompressStream(resName);
    }
    else
    {
        
        if( !FSUtil::exists(txtName) )
        {
            return NULL;
        }
        else
        {
            std::ofstream output(resName.c_str());
            std::ifstream file(txtName.c_str());
            
            file.seekg(0, std::ios_base::end);
            int len = file.tellg();
            file.seekg(0, std::ios_base::beg);
            char* data = (char*)malloc(len);
            file.read(data, len);
            file.close();
            int compressLen = 0;
            char* compressData = izenelib::util::_tc_bzcompress(data, len, &compressLen);
            free(data);
            output.write(compressData, compressLen);
            output.close();
            return decompressStream(resName);
        }
    }
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

NS_IDMLIB_UTIL_END

#endif
