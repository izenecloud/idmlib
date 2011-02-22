#ifndef IDMLIB_NEC_NECITEM_H_
#define IDMLIB_NEC_NECITEM_H_


#include <string>
#include <iostream>
#include <sstream>
#include <idmlib/idm_types.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <util/ustring/UString.h>
#include <am/3rdparty/rde_hash.h>
NS_IDMLIB_NEC_BEGIN
    
class NECItem
{
  typedef std::pair<uint32_t, uint32_t> id2count_t;
  typedef std::pair<izenelib::util::UString, uint32_t> str2count_t;
public:
  NECItem()
  {
  }
  NECItem( const izenelib::util::UString& a, const std::vector<id2count_t>& b, const std::vector<str2count_t>& c, const std::vector<str2count_t>& d)
  :surface(a), id2countList(b), leftTermList(c), rightTermList(d)
  {
  }
  izenelib::util::UString surface;
  std::vector<id2count_t> id2countList;
  std::vector<str2count_t> leftTermList;
  std::vector<str2count_t> rightTermList;
  
public:
  
  void get_all_feature_values(std::vector<std::pair<std::string, double> >& features) const
  {
    features.resize(0);
    if(surface.length()<1) return;
    uint32_t freq = 0;
    for(uint32_t i=0;i<id2countList.size();i++)
    {
      freq += id2countList[i].second;
    }
    if(freq<1) return;
    izenelib::util::UString u_pu = surface.substr(0,1);
    std::string pu;
    u_pu.convertString(pu, izenelib::util::UString::UTF_8);
    std::pair<std::string, double> f_pu("PU|"+pu, 1.0);
    features.push_back(f_pu);
    izenelib::util::UString u_su = surface.substr(surface.length()-1,1);
    std::string su;
    u_su.convertString(su, izenelib::util::UString::UTF_8);
    std::pair<std::string, double> f_su("SU|"+su, 1.0);
    features.push_back(f_su);
    if(surface.length()>1)
    {
      izenelib::util::UString u_pb = surface.substr(0,2);
      std::string pb;
      u_pb.convertString(pb, izenelib::util::UString::UTF_8);
      std::pair<std::string, double> f_pb("PB|"+pb, 1.0);
      features.push_back(f_pb);
      izenelib::util::UString u_sb = surface.substr(surface.length()-2,2);
      std::string sb;
      u_sb.convertString(sb, izenelib::util::UString::UTF_8);
      std::pair<std::string, double> f_sb("SB|"+sb, 1.0);
      features.push_back(f_sb);
    }
    
    izenelib::am::rde_hash<std::string, bool> apps;
    for(uint32_t i=0;i<leftTermList.size();i++)
    {
      double value = (double)(leftTermList[i].second)/freq;
      std::string str_context;
      leftTermList[i].first.convertString(str_context, izenelib::util::UString::UTF_8);
      if(str_context.length()==0) continue;
      std::pair<std::string, double> f_context("LC|"+str_context, value);
      features.push_back(f_context);
      
    }
    
    for(uint32_t i=0;i<rightTermList.size();i++)
    {
      double value = (double)(rightTermList[i].second)/freq;
      std::string str_context;
      rightTermList[i].first.convertString(str_context, izenelib::util::UString::UTF_8);
      if(str_context.length()==0) continue;
      std::pair<std::string, double> f_context("RC|"+str_context, value);
      features.push_back(f_context);
    }
    double len_score = (double) (surface.length())/10.0;
    if(len_score>1.0) len_score = 1.0;
    std::pair<std::string, double> f_len("LENGTH", len_score);
    features.push_back(f_len);
  }
  
  
};

   
NS_IDMLIB_NEC_END



#endif 
