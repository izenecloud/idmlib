#include <idmlib/wiki/entity_extractor.h>

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <util/ustring/UString.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <idmlib/util/idm_analyzer.h>
#include <boost/algorithm/string/case_conv.hpp>
using namespace idmlib::wiki;

bool EntityExtractor::extract(const std::string& input_xml, const std::string& output_file)
{
  idmlib::util::IDMAnalyzer analyzer;
  analyzer.ExtractSpecialChar(true, false);
  std::ofstream ofs(output_file.c_str());
  std::ifstream ifs(input_xml.c_str());
  std::string line;
  izenelib::util::UString left("[[", izenelib::util::UString::UTF_8);
  izenelib::util::UString right("]]", izenelib::util::UString::UTF_8);
  izenelib::util::UString spliter("|", izenelib::util::UString::UTF_8);
  izenelib::util::UString colon(":", izenelib::util::UString::UTF_8);
  uint32_t line_num = 0;
  while(getline( ifs, line))
  {
    line_num++;
    if(line_num%100000==0)
    {
      std::cout<<"Processing line : "<<line_num<<std::endl;
    }
    //TODO if line start with FILE: continuel
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
      izenelib::util::UString l("__blank__", izenelib::util::UString::UTF_8);
      izenelib::util::UString r("__blank__", izenelib::util::UString::UTF_8);
      izenelib::util::UString surface = entity;
      izenelib::util::UString ref = entity;
      if(left_context.length()>0)
      {
        
        la::TermList term_list;
        analyzer.GetTermList(left_context, term_list);
        if(term_list.size()>0)
        {
          l = term_list.back().text_;
//           std::string str;
//           l.convertString(str, izenelib::util::UString::UTF_8);
//           std::cout<<"left context : "<<str<<std::endl;
        }
      }
      if(right_context.length()>0)
      {
        la::TermList term_list;
        analyzer.GetTermList(right_context, term_list);
        if(term_list.size()>0)
        {
          r = term_list.front().text_;
        }
      }
      else
      {
        continue;
      }
      
      uint32_t entity_split = entity.find(spliter);
      if(entity_split!=(uint32_t)-1)
      {
        ref = entity.substr(0, entity_split);
        surface = entity.substr(entity_split+1, entity.length()-entity_split-1);
      }
      //TODO write
      std::string surface_str;
      surface.convertString(surface_str, izenelib::util::UString::UTF_8);
      std::string ref_str;
      ref.convertString(ref_str, izenelib::util::UString::UTF_8);
      std::string l_str;
      l.convertString(l_str, izenelib::util::UString::UTF_8);
      std::string l_str_low = boost::algorithm::to_lower_copy(l_str);
      if(l_str_low=="redirect" || l_str_low=="redirection") continue;
      std::string r_str;
      r.convertString(r_str, izenelib::util::UString::UTF_8);
      ofs<<surface_str<<"\t"<<ref_str<<"\t"<<l_str<<"\t"<<r_str<<std::endl;
      
    }
    
  }
  ifs.close();
  ofs.close();
  return true;
}

