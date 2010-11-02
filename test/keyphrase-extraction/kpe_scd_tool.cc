#include <idmlib/keyphrase-extraction/kpe_algorithm.h>
#include "../TestResources.h"
using namespace idmlib;
using namespace idmlib::kpe;
using namespace idmlib::util;
using namespace boost::filesystem;

void Callback(const izenelib::util::UString& str, uint32_t df, uint8_t score)
{
  std::string s;
  str.convertString(s, izenelib::util::UString::UTF_8);
  std::cout<<"["<<s<<"\t"<<df<<"]"<<std::endl;
}

int main(int ac, char** av)
{
  std::string resource_path(av[1]);
  std::string scd_file(av[2]);
  typedef KPEOutput<true, false, false> OutputType ;
  IDMAnalyzer* analyzer = new IDMAnalyzer(WISEKMA_KNOWLEDGE);
  std::string work_dir = "./kpe_scd_working";
  boost::filesystem::remove_all(work_dir);
  KPEAlgorithm<OutputType>* kpe = new KPEAlgorithm<OutputType>(work_dir, analyzer, &Callback);
  kpe->load(resource_path);
  uint32_t doc_id = 0;
  std::ifstream ifs( scd_file.c_str() );
  std::string content = "";
  std::string line;
  bool in_content = false;
  while ( getline ( ifs,line ) )
  {
    content = line;
    std::string lower = boost::to_lower_copy(line);
    if( lower.find("<docid>") == 0 )
    {
      doc_id++;
      in_content = false;
    }
    else if(lower.find("<title>")==0)
    {
      content = line.substr(7, line.length()-7 );
      in_content = true;
    }
    else if(lower.find("<content>")==0)
    {
      content = line.substr(9, line.length()-9 );
      in_content = true;
    }
    else if(lower.find("<")==0)
    {
      in_content = false;
    }
    
    if( in_content )
    {
      izenelib::util::UString uc(content, izenelib::util::UString::UTF_8);
      kpe->insert( uc, doc_id);
    }
  }
  
  kpe->close();
  delete kpe;
  delete analyzer;
  boost::filesystem::remove_all(work_dir);
  return 0;

  
}
