#ifndef IDMLIB_WIKI_FEATUREITEM_H_
#define IDMLIB_WIKI_FEATUREITEM_H_


#include <string>
#include <iostream>
#include <sstream>
#include <idmlib/idm_types.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <util/ustring/UString.h>
#include <am/3rdparty/rde_hash.h>
NS_IDMLIB_WIKI_BEGIN
    
class FeatureItem
{
public:
  typedef std::pair<izenelib::util::UString, izenelib::util::UString> ContextType;
  izenelib::util::UString surface;
  std::vector<ContextType> context_list;
  int type;
  
public:
  
  void get_all_feature_values(std::vector<std::pair<std::string, double> >& features)
  {
    features.resize(0);
    if(surface.length()<1) return;
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
    if(context_list.size()==0) return;
    uint32_t count = context_list.size();
    {
      izenelib::am::rde_hash<izenelib::util::UString, uint32_t> count_map;
      for(uint32_t i=0;i<count;i++)
      {
        izenelib::util::UString context = context_list[i].first;
        uint32_t* p_count = count_map.find(context);
        if(p_count==NULL)
        {
          count_map.insert(context, 1);
        }
        else
        {
          *p_count = (*p_count)+1;
        }
      }
      izenelib::am::rde_hash<izenelib::util::UString , bool> apps;
      for(uint32_t i=0;i<count;i++)
      {
        izenelib::util::UString context = context_list[i].first;
        if(apps.find(context)!=NULL) continue;
        uint32_t* p_count = count_map.find(context);
        double value = (double)(*p_count)/count;
        std::string str_context;
        context.convertString(str_context, izenelib::util::UString::UTF_8);
        std::pair<std::string, double> f_context("LC|"+str_context, value);
        features.push_back(f_context);
        apps.insert(context, 1);
      }
    }
    {
      izenelib::am::rde_hash<izenelib::util::UString, uint32_t> count_map;
      for(uint32_t i=0;i<count;i++)
      {
        izenelib::util::UString context = context_list[i].second;
        uint32_t* p_count = count_map.find(context);
        if(p_count==NULL)
        {
          count_map.insert(context, 1);
        }
        else
        {
          *p_count = (*p_count)+1;
        }
      }
      izenelib::am::rde_hash<izenelib::util::UString , bool> apps;
      for(uint32_t i=0;i<count;i++)
      {
        izenelib::util::UString context = context_list[i].second;
        if(apps.find(context)!=NULL) continue;
        uint32_t* p_count = count_map.find(context);
        double value = (double)(*p_count)/count;
        std::string str_context;
        context.convertString(str_context, izenelib::util::UString::UTF_8);
        std::pair<std::string, double> f_context("RC|"+str_context, value);
        features.push_back(f_context);
        apps.insert(context, 1);
      }
    }
  }
  
  std::string key() const
  {
    std::string r;
    surface.convertString(r, izenelib::util::UString::UTF_8);
    return r;
  }
  
//   std::string ref_str() const
//   {
//     std::string r;
//     ref.convertString(r, izenelib::util::UString::UTF_8);
//     return r;
//   }
  
  std::string ToString() const
  {
    //surface, context_list, type
    std::stringstream ss;
    ss<<key()<<"\t"<<context_list.size();
    for(uint32_t i=0;i<context_list.size();i++)
    {
      std::string left;
      context_list[i].first.convertString(left, izenelib::util::UString::UTF_8);
      std::string right;
      context_list[i].second.convertString(right, izenelib::util::UString::UTF_8);
      ss<<"\t"<<left<<","<<right;
    }
    ss<<"\t"<<type;
    return ss.str();
  }
  
  void Parse(const std::string& str)
  {
    std::vector<std::string> vec_value;
    boost::algorithm::split( vec_value, str, boost::algorithm::is_any_of("\t") );
    surface = izenelib::util::UString(vec_value[0], izenelib::util::UString::UTF_8);
    uint32_t context_num = boost::lexical_cast<uint32_t>(vec_value[1]);
    for(uint32_t i=0;i<context_num;i++)
    {
      std::string context_str = vec_value[2+i];
      std::vector<std::string> vec;
      boost::algorithm::split( vec, context_str, boost::algorithm::is_any_of(",") );
      std::string left = vec[0];
      std::string right = vec[1];
      if(left=="__blank__") left = "";
      if(right=="__blank__") right = "";
      if(left=="__comma__") left = ",";
      if(right=="__comma__") right = ",";
      ContextType context(izenelib::util::UString(left, izenelib::util::UString::UTF_8), 
                           izenelib::util::UString(right, izenelib::util::UString::UTF_8));
      context_list.push_back(context);
    }
    type = boost::lexical_cast<int>(vec_value.back());
  }
};

   
NS_IDMLIB_WIKI_END



#endif 
