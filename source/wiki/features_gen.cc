#include <idmlib/wiki/features_gen.h>
#include <idmlib/wiki/feature_item.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <util/ustring/UString.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <idmlib/util/idm_analyzer.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/regex.hpp>
using namespace idmlib::wiki;

FeaturesGen::FeaturesGen(const std::string& dir):dir_(dir)
{
//   std::string file = dir_+"/cs_ct";
//   std::ifstream ifs(file.c_str());
//   if(ifs.fail())
//   {
//     std::cout<<"can not find "<<file<<std::endl;
//   }
//   else
//   {
//     std::string line;
//     while( getline( ifs, line) )
//     {
//       boost::algorithm::trim( line );
//       izenelib::util::UString uline(line, izenelib::util::UString::UTF_8);
//       t2s_map_.insert( uline[2], uline[0] );
//     }
//   }
//   ifs.close();
}

bool FeaturesGen::gen_redirect()
{
  std::string redirect_file = dir_+"/redirect";
  std::ifstream ifs(redirect_file.c_str());
  if(ifs.fail())
  {
    std::cout<<"can not find "<<redirect_file<<std::endl;
    return false;
  }
  std::string line;
  while(getline( ifs, line))
  {
    boost::algorithm::trim(line);
    std::vector<std::string> vec_value;
    boost::algorithm::split( vec_value, line, boost::algorithm::is_any_of("\t") );
    if(vec_value.size()!=2)
    {
      std::cout<<"invalid row : "<<line<<std::endl;
      continue;
    }
    redirect_.insert(vec_value[0], vec_value[1]);
    std::vector<std::string>* images = redirect_image_.find(vec_value[1]);
    if(images==NULL)
    {
      std::vector<std::string> img;
      img.push_back(vec_value[0]);
      redirect_image_.insert(vec_value[1], img);
    }
    else
    {
      images->push_back(vec_value[0]);
    }
  }
  ifs.close();
  return true;
  
  
}

bool FeaturesGen::gen_types_(const std::string& file, int type)
{
  std::ifstream ifs(file.c_str());
  if(ifs.fail())
  {
    std::cout<<"can not find "<<file<<std::endl;
    return false;
  }
  std::string line;
  while(getline( ifs, line))
  {
    boost::algorithm::trim(line);
    std::vector<std::string> vec_value;
    boost::algorithm::split( vec_value, line, boost::algorithm::is_any_of(",") );
    std::string entity = "";
    if(vec_value.size()==2)
    {
      entity = vec_value[1];
    }
    else if(vec_value.size()==1)
    {
      entity = line;
    }
    else
    {
      std::cout<<"invalid row : "<<line<<std::endl;
      continue;
    }
    if(types_.find(entity)!=NULL)
    {
      continue;
    }
    std::string image = entity;
    std::string* c_image = redirect_.find(entity);
    if(c_image!=NULL)
    {
      image = *c_image;
    }
    types_.insert(image, type);
    std::vector<std::string>* keys = redirect_image_.find(image);
    if(keys!=NULL)
    {
      for(uint32_t i=0;i<keys->size();i++)
      {
        types_.insert((*keys)[i], type);
      }
    }
  }
  return true;
}

bool FeaturesGen::gen_ner_types()
{
  //peop as 1, loc as 2, org as 3
  {
    std::string file = dir_+"/peop";
    if(!gen_types_(file, 1))
    {
      std::cout<<"gen "<<file<<" failed"<<std::endl;
      return false;
    }
  }
  {
    std::string file = dir_+"/loc";
    if(!gen_types_(file, 2))
    {
      std::cout<<"gen "<<file<<" failed"<<std::endl;
      return false;
    }
  }
  {
    std::string file = dir_+"/org";
    if(!gen_types_(file, 3))
    {
      std::cout<<"gen "<<file<<" failed"<<std::endl;
      return false;
    }
  }
  return true;
}

bool FeaturesGen::output(const std::string& output_file)
{
  std::string file = dir_+"/entities";
  std::ifstream ifs(file.c_str());
  if(ifs.fail())
  {
    std::cout<<"can not find "<<file<<std::endl;
    return false;
  }
  
  std::ofstream ofs(output_file.c_str());
  if(ofs.fail())
  {
    std::cout<<"can not open "<<output_file<<std::endl;
    return false;
  }
  std::string line;
  izenelib::am::rde_hash<izenelib::util::UString, FeatureItem> feature_item_map;
  std::vector<izenelib::util::UString> surface_list;
  while(getline( ifs, line))
  {
    boost::algorithm::trim(line);
    std::vector<std::string> vec_value;
    boost::algorithm::split( vec_value, line, boost::algorithm::is_any_of("\t") );
    if(vec_value.size()!=4)
    {
      std::cout<<"invalid row : "<<line<<std::endl;
      continue;
    }
    izenelib::util::UString surface(vec_value[0], izenelib::util::UString::UTF_8);
    t2s_(surface);
    std::string ref = vec_value[1];
    std::string left = vec_value[2];
    std::string right = vec_value[3];
    if(left=="__blank__") left = "";
    if(right=="__blank__") right = "";
    izenelib::util::UString uleft(left, izenelib::util::UString::UTF_8);
    izenelib::util::UString uright(right, izenelib::util::UString::UTF_8);
    t2s_(uleft);
    t2s_(uright);
    std::pair<izenelib::util::UString, izenelib::util::UString> context(uleft, uright);
    int type = 0;
    FeatureItem* fitem = feature_item_map.find(surface);
    if(fitem!=NULL)
    {
      type = fitem->type;
      if(type==0)
      {
        int* p_type = types_.find(ref);
        if(p_type!=NULL)
        {
          type = *p_type;
        }
        fitem->type = type;
      }
      fitem->context_list.push_back(context);
    }
    else
    {
      FeatureItem f_item;
      f_item.surface = surface;
      f_item.context_list.push_back(context);
      int* p_type = types_.find(ref);
      if(p_type!=NULL)
      {
        type = *p_type;
      }
      f_item.type = type;
      feature_item_map.insert(surface, f_item);
      surface_list.push_back(surface);
    }
  }
  ifs.close();
  for(uint32_t i=0;i<surface_list.size();i++)
  {
    FeatureItem* fitem = feature_item_map.find(surface_list[i]);
    ofs<<fitem->ToString()<<std::endl;
  }
  ofs.close();
  return true;
}

bool FeaturesGen::gen_all(const std::string& output_file)
{
  if(!gen_redirect())
  {
    std::cout<<"gen_redirect failed"<<std::endl;
    return false;
  }
  if(!gen_ner_types())
  {
    std::cout<<"gen_ner_types failed"<<std::endl;
    return false;
  }
  if(!output(output_file))
  {
    std::cout<<"output failed"<<std::endl;
    return false;
  }
  return true;
}

void FeaturesGen::t2s_(izenelib::util::UString& ustr)
{
  return;
  for(uint32_t i=0;i<ustr.length();i++)
    {
      izenelib::util::UCS2Char* p_char = t2s_map_.find(ustr[i]);
      if( p_char != NULL )
      {
        ustr[i] = *p_char;
      }
    }
}

