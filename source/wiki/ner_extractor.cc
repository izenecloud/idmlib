#include <idmlib/wiki/ner_extractor.h>

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

bool NerExtractor::extract_from_xml(const std::string& input_xml, const std::string& title_regex)
{
  std::ifstream ifs(input_xml.c_str());
  std::string line;
  izenelib::util::UString left("[[", izenelib::util::UString::UTF_8);
  izenelib::util::UString right("]]", izenelib::util::UString::UTF_8);
  izenelib::util::UString spliter("|", izenelib::util::UString::UTF_8);
  izenelib::util::UString colon(":", izenelib::util::UString::UTF_8);
  std::string page_end = "</page>";
  std::string title = "";
  uint32_t line_num = 0;
  bool valid_title = false;
  boost::regex reg_exp(title_regex);
  while(getline( ifs, line))
  {
    line_num++;
    if(line_num%100000==0)
    {
//       std::cout<<"Processing line : "<<line_num<<std::endl;
    }
    boost::algorithm::trim(line);
    if(line == page_end )
    {
      title = "";
      valid_title = false;
      continue;
    }
    if( boost::starts_with(line, "<title>") && boost::ends_with(line, "</title>") )
    {
      title = line.substr(7, line.length()-15);
//       std::cout<<"find title : "<<title<<std::endl;
      if( boost::regex_match(title, reg_exp) )
      {
        std::cout<<"Match! : "<<title<<std::endl;
        valid_title = true;
      }
      else
      {
        valid_title = false;
      }
    }
    if(!valid_title) continue;
    izenelib::util::UString text(line, izenelib::util::UString::UTF_8);
    std::size_t pos = 0;
    while(true)
    {
      uint32_t left_pos = text.find(left, pos);
      if( left_pos==(uint32_t)-1 ) break;
//       std::cout<<"find left in line "<<line<<" @@@ "<<left_pos<<std::endl;
      uint32_t right_pos = text.find(right, left_pos);
      if( right_pos==(uint32_t)-1 ) break;
      izenelib::util::UString left_context = text.substr(pos, left_pos-pos);
      izenelib::util::UString right_context = text.substr(right_pos+2, text.length()-right_pos-2);
      izenelib::util::UString entity = text.substr(left_pos+2, right_pos-left_pos-2);
      pos = right_pos;
      if(entity.find(colon)!=(uint32_t)-1) continue;
      
      izenelib::util::UString surface = entity;
      izenelib::util::UString ref = entity;
      
      
      uint32_t entity_split = entity.find(spliter);
      if(entity_split!=(uint32_t)-1)
      {
        ref = entity.substr(0, entity_split);
        surface = entity.substr(entity_split+1, entity.length()-entity_split-1);
      }
      //TODO write
      if(surface.length()>1)
      {
        std::string surface_str;
        surface.convertString(surface_str, izenelib::util::UString::UTF_8);
        if(boost::ends_with(surface_str, "列表")) continue;
        std::string ref_str;
        ref.convertString(ref_str, izenelib::util::UString::UTF_8);
        std::cout<<surface_str<<","<<ref_str<<std::endl;
      }
      
    }
    
  }
  ifs.close();
  return true;
}


bool NerExtractor::extract_from_catelink(const std::string& input_file, const std::string& category_regex, const std::string& page_regex)
{
  std::ifstream ifs(input_file.c_str());
  std::string line;
  izenelib::util::UString left("[[", izenelib::util::UString::UTF_8);
  izenelib::util::UString right("]]", izenelib::util::UString::UTF_8);
  izenelib::util::UString spliter("|", izenelib::util::UString::UTF_8);
  izenelib::util::UString colon(":", izenelib::util::UString::UTF_8);
  uint32_t line_num = 0;
  boost::regex reg_exp(category_regex);
  boost::regex* page_reg = NULL;
  if(page_regex!="")
  {
    page_reg = new boost::regex(page_regex);
  }
  while(getline( ifs, line))
  {
    line_num++;
    if(line_num%100000==0)
    {
//       std::cout<<"Processing line : "<<line_num<<std::endl;
    }
    std::vector<std::string> vec_value;
    boost::algorithm::split( vec_value, line, boost::algorithm::is_any_of("\t") );
    if(vec_value.size()!=2)
    {
      std::cout<<"invalid row : "<<line<<std::endl;
      continue;
    }
    std::string page = vec_value[0];
    if(boost::ends_with(page, "列表")) continue;
    izenelib::util::UString upage(page, izenelib::util::UString::UTF_8);
    if( page.find(":")!=std::string::npos ) continue;
    if(upage.length()<2) continue;
    if(upage.isAlphaChar(0)) continue;
    std::string category = vec_value[1];
    if( boost::regex_match(category, reg_exp) )
    {
      if(page_reg!=NULL)
      {
        if( boost::regex_match(page, *page_reg) )
        {
          std::cout<<page<<std::endl;
        }
      }
      
    }
  }
  ifs.close();
  return true;
}

