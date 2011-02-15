#include <idmlib/wiki/category_extractor.h>

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <util/ustring/UString.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <idmlib/util/idm_analyzer.h>
#include <boost/algorithm/string/split.hpp>
using namespace idmlib::wiki;

bool CategoryExtractor::extract(const std::string& input_xml, const std::string& output_file)
{
  std::ofstream ofs(output_file.c_str());
  std::ifstream ifs(input_xml.c_str());
  std::string line;
  izenelib::util::UString left("[[", izenelib::util::UString::UTF_8);
  izenelib::util::UString right("]]", izenelib::util::UString::UTF_8);
  izenelib::util::UString spliter("|", izenelib::util::UString::UTF_8);
  izenelib::util::UString colon(":", izenelib::util::UString::UTF_8);
  std::string page_end = "</page>";
  std::string title = "";
  izenelib::util::UString utitle;
  uint32_t line_num = 0;
  bool valid_title = false;
  boost::regex cate_exp("\\[\\[Category:(.+?)(\\|.*)?\\]\\]");
  boost::cmatch matches;
  while(getline( ifs, line))
  {
    line_num++;
    if(line_num%100000==0)
    {
      std::cout<<"Processing line : "<<line_num<<std::endl;
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
      if( title.find(":")==std::string::npos ) 
      {
//         std::cout<<"Match! : "<<title<<std::endl;
        utitle = izenelib::util::UString(title, izenelib::util::UString::UTF_8);
        valid_title = true;
      }
      else
      {
        valid_title = false;
      }
    }
    if(!valid_title) continue;
    if(boost::regex_match(line.c_str(), matches, cate_exp))
    {
      std::string category(matches[1].first, matches[1].second);
      ofs<<title<<"\t"<<category<<std::endl;
    }
  }
  ifs.close();
  ofs.close();
  return true;
}

