#include <idmlib/wiki/ner_extractor.h>
#include <boost/regex.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
using namespace idmlib::wiki;

int main(int argc, char** argv)
{
  std::string file(argv[1]);
  std::string rule(argv[2]);
//   std::cout<<"file : "<<file<<std::endl;
//   std::cout<<"rule : "<<rule<<std::endl;
  bool is_xml = false;
  if( boost::ends_with(file, ".xml") )
  {
    is_xml = true;
  }
  
  if(is_xml)
  {
    NerExtractor::extract_from_xml(file, rule);
  }
  else
  {
    std::string page_rule = "";
    if(argc>3)
    {
      page_rule = std::string(argv[3]);
    }
    NerExtractor::extract_from_catelink(file, rule, page_rule);
  }
  return 0;
//   std::string xml_file(argv[1]);
//   std::string catelink_file(argv[2]);
//   std::string output_dir(argv[3]);
//   boost::filesystem::remove_all(output_dir);
//   boost::filesystem::create_directories(output_dir);
//   
//   std::vector<std::string> peop_rule_xml;
//   peop_rule_xml.push_back("^人名表 \\([A-Z]\\)$");
//   
//   std::vector<std::string> peop_rule_catelink;
//   peop_rule_catelink.push_back("^.*年逝世$");
//   peop_rule_catelink.push_back("^.*年出生$");
//   peop_rule_catelink.push_back("^.*画家$");
//   peop_rule_catelink.push_back("^.*校友$");
//   peop_rule_catelink.push_back("^.*学家$");
//   
//   std::vector<std::string> loc_rule_xml;
//   loc_rule_xml.push_back("^.*地名列表(/[A-Z])?$");
//   
//   std::vector<std::string> loc_rule_catelink;
//   peop_rule_catelink.push_back("^.*广场$");
//   
//   std::vector<std::string> org_rule_xml;
//   
//   std::vector<std::string> org_rule_catelink;
//   org_rule_catelink.push_back("^.*机构$");
//   org_rule_catelink.push_back("^.+组织$");
//   org_rule_catelink.push_back("^.*院校$");
//   org_rule_catelink.push_back("^.*大学$");
//   org_rule_catelink.push_back("^.*学校$");
//   org_rule_catelink.push_back("^.*公司$");
//   org_rule_catelink.push_back("^.*企业$");
//   org_rule_catelink.push_back("^.*会议$");
//   org_rule_catelink.push_back("^.*电视台$");
//   org_rule_catelink.push_back("^.*科学院$");
//   org_rule_catelink.push_back("^.*博物馆$");
//   org_rule_catelink.push_back("^.*集团$");
//   org_rule_catelink.push_back("^.*委员会$");
//   org_rule_catelink.push_back("^.*银行$");
//   
//   std::string output_file = "";
//   
//   {
//     int index = 1;
//     for(uint32_t i=0;i<peop_rule_xml.size();i++)
//     {
//       output_file = output_dir+"/"+boost::lexical_cast<std::string>(index)+".peop";
//       NerExtractor::extract_from_xml(xml_file, output_file, peop_rule_xml[i]);
//       index++;
//     }
//     for(uint32_t i=0;i<peop_rule_catelink.size();i++)
//     {
//       output_file = output_dir+"/"+boost::lexical_cast<std::string>(index)+".peop";
//       NerExtractor::extract_from_catelink(catelink_file, output_file, peop_rule_catelink[i]);
//       index++;
//     }
//   }
//   
//   {
//     int index = 1;
//     for(uint32_t i=0;i<loc_rule_xml.size();i++)
//     {
//       output_file = output_dir+"/"+boost::lexical_cast<std::string>(index)+".loc";
//       NerExtractor::extract_from_xml(xml_file, output_file, loc_rule_xml[i]);
//       index++;
//     }
//     for(uint32_t i=0;i<loc_rule_catelink.size();i++)
//     {
//       output_file = output_dir+"/"+boost::lexical_cast<std::string>(index)+".loc";
//       NerExtractor::extract_from_catelink(catelink_file, output_file, loc_rule_catelink[i]);
//       index++;
//     }
//   }
//   
//   {
//     int index = 1;
//     for(uint32_t i=0;i<org_rule_xml.size();i++)
//     {
//       output_file = output_dir+"/"+boost::lexical_cast<std::string>(index)+".org";
//       NerExtractor::extract_from_xml(xml_file, output_file, org_rule_xml[i]);
//       index++;
//     }
//     for(uint32_t i=0;i<org_rule_catelink.size();i++)
//     {
//       output_file = output_dir+"/"+boost::lexical_cast<std::string>(index)+".org";
//       NerExtractor::extract_from_catelink(catelink_file, output_file, org_rule_catelink[i]);
//       index++;
//     }
//   }

}
