#include <idmlib/tdt/temporal_kpe.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <util/scd_parser.h>
#include <idmlib/similarity/term_similarity.h>
#include "../TestResources.h"
using namespace idmlib;
using namespace idmlib::tdt;
using namespace idmlib::util;
using namespace boost::filesystem;
namespace po = boost::program_options;
typedef idmlib::sim::TermSimilarity<> TermSimilarityType;
typedef TermSimilarityType::SimTableType SimTableType;
int main(int ac, char** av)
{
  po::options_description desc("Allowed options");
  desc.add_options()
    ("help", "produce help message")
    ("resource-path,R", po::value<std::string>(), "system resource path")
    ("scd-path,S", po::value<std::string>(), "scd directory to be processed")
    ("output-file,O", po::value<std::string>(), "output key-phrases file, include the string representation and df")
    ("kma-path,K", po::value<std::string>(), "if we want to process Korean collection, specify this kma path")
    ("working-path,W", po::value<std::string>(), "temp working path used for kpe, default: ./kpe_scd_working")
    ("max-doc,M", po::value<uint32_t>(), "max doc count which will be processed.")
    ("exception-text-file,E", po::value<std::string>(), "exception text list")
  ;
  std::string default_working_path = "./temporal_kpe_working";
  izenelib::util::UString::EncodingType encoding = izenelib::util::UString::UTF_8;
  po::variables_map vm;
  po::store(po::parse_command_line(ac, av, desc), vm);
  po::notify(vm); 
  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return 1;
  }
  std::string resource_path;
  if (vm.count("resource-path")) {
    resource_path = vm["resource-path"].as<std::string>();
    std::cout << "resource-path: " << resource_path <<std::endl;
  } 
  else {
    std::cerr<<"resource-path not set"<<std::endl;
    std::cerr << desc << std::endl;
    return -1;
  }
  
  std::vector<izenelib::util::UString> exception_text_list;
  
  std::string exception_file;
  if (vm.count("exception-text-file")) {
    std::string exception_file = vm["exception-text-file"].as<std::string>();
    std::cout << "exception-text-file: " << exception_file <<std::endl;
    
    std::ifstream ifs(exception_file.c_str());
    std::string line;
    while ( getline ( ifs,line ) )
    {
        exception_text_list.push_back(izenelib::util::UString(line, izenelib::util::UString::UTF_8));
    }
    ifs.close();
  } 
 
  std::string scd_path;
  std::vector<std::string> scdfile_list;
  if (vm.count("scd-path")) {
    scd_path = vm["scd-path"].as<std::string>();
    try{
      if (!is_directory(scd_path))
      {
          std::cerr<<scd_path<<"is not directory"<<std::endl;
          return -1;
      }
      directory_iterator kItrEnd;
      for (directory_iterator itr(scd_path); itr != kItrEnd; ++itr)
      {
          std::string file_name = itr->path().filename();
          if (izenelib::util::ScdParser::checkSCDFormat(file_name) )
          {
            izenelib::util::SCD_TYPE scd_type = izenelib::util::ScdParser::checkSCDType(file_name);
            if( scd_type == izenelib::util::INSERT_SCD ||scd_type == izenelib::util::UPDATE_SCD )
            {
              scdfile_list.push_back(itr->path().string() );
            }
          }
      }
    }
    catch(std::exception& ex)
    {
      std::cerr<<"fs error"<<std::endl;
      return -1;
    }
    if( scdfile_list.size()==0 )
    {
      std::cout<<"no scd file under "<<scd_path<<std::endl;
      return 1;
    }
    std::cout << "scd-path: " << scd_path <<std::endl;
  } 
  else {
    std::cerr<<"scd-path not set"<<std::endl;
    std::cerr << desc << std::endl;
    return -1;
  }
  
  
  
  std::string output_file;
  if (vm.count("output-file")) {
    output_file = vm["output-file"].as<std::string>();
    std::cout << "output-file: " << output_file <<std::endl;
  } 
  else {
    std::cerr<<"output-file not set"<<std::endl;
    std::cerr << desc << std::endl;
    return -1;
  }
  
  
  
  idmlib::util::IDMAnalyzer* analyzer = NULL;
  if (vm.count("kma-path")) {
    std::string kma_path = vm["kma-path"].as<std::string>();
    std::cout << "kma-path: " << kma_path <<std::endl;
    analyzer = new idmlib::util::IDMAnalyzer(kma_path);
  } else {
    std::cout << "kma-path not set"<<std::endl;
    analyzer = new idmlib::util::IDMAnalyzer();
  }
  
  std::string working_path;
  if (vm.count("working-path")) {
    working_path = vm["working-path"].as<std::string>();
    std::cout << "working-path: " << working_path <<std::endl;
  } 
  else {
    std::cout<<"working-path not set, use default "<<default_working_path<<std::endl;
    working_path = default_working_path;
  }
  try
  {
      boost::filesystem::remove_all(working_path);
      boost::filesystem::create_directories(working_path);
  }
  catch(std::exception& ex)
  {
      std::cerr<<"delete "<<working_path<<" error"<<std::endl;
      return -1;
  }
  
  uint32_t max_doc = 0;
  if (vm.count("max-doc")) {
    max_doc = vm["max-doc"].as<uint32_t>();
    std::cout << "max-doc: " << max_doc <<std::endl;
  } 

  std::string kpe_resource_path = resource_path+"/kpe";
  std::string rig_resource_path = resource_path+"/sim/rig";
  if( !analyzer->LoadT2SMapFile(kpe_resource_path+"/cs_ct") )
  {
    return -1;
  }
  analyzer->ExtractSymbols();
  DateRange date_range;
  date_range.start = boost::gregorian::from_string("2011-03-01");
  date_range.end = boost::gregorian::from_string("2011-04-30");
  
  
  SimTableType* table = new SimTableType(working_path+"/sim_table");
  if(!table->Open())
  {
      std::cerr<<"sim table open error"<<std::endl;
      return -1;
  }
  
  
  TermSimilarityType* sim = new TermSimilarityType(working_path+"/sim", rig_resource_path, table, 5, 0.2 );
  TemporalKpe* kpe = new TemporalKpe(working_path, analyzer, date_range, sim);
  if( !kpe->load(kpe_resource_path) )
  {
    return -1;
  }
  
  for(uint32_t i=0;i<exception_text_list.size();i++)
  {
      kpe->AddException(exception_text_list[i]);
  }
  
//   kpe->set_tracing(izenelib::util::UString("油价", izenelib::util::UString::UTF_8));
  uint32_t docid = 0;
  
  for(uint32_t i=0;i<scdfile_list.size();i++)
  {
    std::string scd_file = scdfile_list[i];
    izenelib::util::ScdParser scd_parser(encoding);
    if(!scd_parser.load(scd_file) )
    {
      std::cerr<<"load scd file failed."<<std::endl;
      return -1;
    }
    izenelib::util::ScdParser::iterator it = scd_parser.begin();
    
    for( ;it!= scd_parser.end();++it )
    {
      izenelib::util::SCDDocPtr doc = (*it);
      if(!doc)
      {
        std::cerr<<"scd parsing error"<<std::endl;
        break;
      }
      std::vector<std::pair<izenelib::util::UString, izenelib::util::UString> >::iterator p;
      bool valid = true;
      for (p = doc->begin(); p != doc->end(); p++)
      {
          if( p->second == izenelib::util::UString("null", encoding))
          {
              valid = false;
              break;
          }
          if( p->second == izenelib::util::UString("(null)", encoding))
          {
              valid = false;
              break;
          }
      }
      if(!valid) continue;
      izenelib::util::UString title;
      izenelib::util::UString content;
      std::string tdt_str = "1";
//       uint32_t time = 0;
      boost::gregorian::date date;
      for (p = doc->begin(); p != doc->end(); p++)
      {
        izenelib::util::UString property_name = p->first;
        property_name.toLowerString();
        if( property_name == izenelib::util::UString("docid", encoding) )
        {
          docid++;
          if( max_doc>0 && docid > max_doc ) break;
          if( docid % 1000 == 0 )
          {
            std::cout<<"Processing "<<docid<<std::endl;
          }
        }
        else if( property_name == izenelib::util::UString("title", encoding))
        {
            title = p->second;
        }
        else if( property_name == izenelib::util::UString("content", encoding))
        {
            content = p->second;
        }
        else if( property_name == izenelib::util::UString("date", encoding))
        {
            std::string date_str;
            p->second.convertString(date_str, izenelib::util::UString::UTF_8);
//             std::cout<<docid<<" "<<date_str<<std::endl;
            date = boost::gregorian::from_string(date_str);
            
            
//             std::cout<<"time:"<<time<<std::endl;
        }
        else if( property_name == izenelib::util::UString("tdt", encoding))
        {
            p->second.convertString(tdt_str, izenelib::util::UString::UTF_8);
//             std::cout<<docid<<" "<<date_str<<std::endl;
            
        }
      }
      if(tdt_str=="1" && date_range.Contains(date))
      {
        kpe->Insert(date, docid, title, content);
      }
    }
  }
  kpe->close();
  delete kpe;
  delete analyzer;
//   boost::filesystem::remove_all(working_path);
  return 0;

  
}
