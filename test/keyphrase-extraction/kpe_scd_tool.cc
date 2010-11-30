#include <idmlib/keyphrase-extraction/kpe_algorithm.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <util/scd_parser.h>

#include "../TestResources.h"
using namespace idmlib;
using namespace idmlib::kpe;
using namespace idmlib::util;
using namespace boost::filesystem;
namespace po = boost::program_options;


class FileKPEWriter
{
 public:
  FileKPEWriter(const std::string& file,izenelib::util::UString::EncodingType encoding)
  :ofs_(file.c_str()), encoding_(encoding)
  {
  }
  
  ~FileKPEWriter()
  {
    ofs_.close();
  }
  
  void Callback(const izenelib::util::UString& str, uint32_t df, uint8_t score)
  {
    std::string s;
    str.convertString(s, encoding_);
    ofs_<<s<<","<<df<<std::endl;
  }
  
 private:
  std::ofstream ofs_;
  izenelib::util::UString::EncodingType encoding_;
};

int main(int ac, char** av)
{
  po::options_description desc("Allowed options");
  desc.add_options()
    ("help", "produce help message")
    ("resource-path,R", po::value<std::string>(), "KPE resource path")
    ("scd-path,S", po::value<std::string>(), "scd directory to be processed")
    ("property,P", po::value<std::string>(), "properties to be processed, seperate with comma(Title,Content), case insensitive")
    ("output-file,O", po::value<std::string>(), "output key-phrases file, include the string representation and df")
    ("kma-path,K", po::value<std::string>(), "if we want to process Korean collection, specify this kma path")
    ("working-path,W", po::value<std::string>(), "temp working path used for kpe, default: ./kpe_scd_working")
    ("max-doc,M", po::value<uint32_t>(), "max doc count which will be processed.")
  ;
  std::string default_working_path = "./kpe_scd_working";
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
  
  izenelib::am::rde_hash<izenelib::util::UString, bool> properties;
  if (vm.count("property")) {
    std::string property_str = vm["property"].as<std::string>();
    std::vector<std::string> property_vec;
    boost::algorithm::split( property_vec, property_str, boost::algorithm::is_any_of(",") );
    std::vector<std::string> nonempty_property_vec;
    for(uint32_t i=0;i<property_vec.size();i++)
    {
      if( property_vec[i] != "")
      {
        nonempty_property_vec.push_back(property_vec[i]);
        izenelib::util::UString property_ustr(property_vec[i], encoding);
        property_ustr.toLowerString();
        properties.insert(property_ustr, 1);
      }
    }
    if( nonempty_property_vec.size()==0)
    {
      std::cerr<<"property not set correctly"<<std::endl;
      std::cerr << desc << std::endl;
      return -1;
    }
    for(uint32_t i=0;i<nonempty_property_vec.size();i++)
    {
      std::cout<<"Process property: "<<nonempty_property_vec[i]<<std::endl;
    }
  } 
  else {
    std::cerr<<"property not set"<<std::endl;
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
    try
    {
      boost::filesystem::remove_all(working_path);
    }
    catch(std::exception& ex)
    {
      std::cerr<<"delete "<<working_path<<" error"<<std::endl;
      return -1;
    }
    std::cout << "working-path: " << working_path <<std::endl;
  } 
  else {
    std::cout<<"working-path not set, use default "<<default_working_path<<std::endl;
    working_path = default_working_path;
  }
  
  uint32_t max_doc = 0;
  if (vm.count("max-doc")) {
    max_doc = vm["max-doc"].as<uint32_t>();
    std::cout << "max-doc: " << max_doc <<std::endl;
  } 

  typedef KPEOutput<true, false, false> OutputType ;
  typedef OutputType::function_type function_type ;
  if( !analyzer->LoadT2SMapFile(resource_path+"/cs_ct") )
  {
    return -1;
  }
  
  FileKPEWriter file_writer(output_file, encoding);
  function_type callback_func = boost::bind( &FileKPEWriter::Callback, &file_writer, _1, _2, _3);
  KPEAlgorithm<OutputType>* kpe = new KPEAlgorithm<OutputType>(working_path, analyzer, callback_func);
  if( !kpe->load(resource_path) )
  {
    return -1;
  }
  
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
    
    while( it!= scd_parser.end() )
    {
      izenelib::util::SCDDocPtr doc = (*it);
      if(!doc)
      {
        std::cerr<<"scd parsing error"<<std::endl;
        break;
      }
      std::vector<std::pair<izenelib::util::UString, izenelib::util::UString> >::iterator p;
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
        else if( properties.find( property_name) != NULL)
        {
          kpe->insert( p->second, docid);
        }
      }
      ++it;
    }
  }
 
  kpe->close();
  delete kpe;
  delete analyzer;
  boost::filesystem::remove_all(working_path);
  return 0;

  
}
