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
  OutputType output(&Callback);
  IDMAnalyzer* analyzer = new IDMAnalyzer(WISEKMA_KNOWLEDGE);
  std::string work_dir = "./kpe_scd_working";
  boost::filesystem::remove_all(work_dir);
  KPEAlgorithm<OutputType>* kpe = new KPEAlgorithm<OutputType>(work_dir, resource_path, analyzer, output);
  uint32_t doc_id = 1;
  std::ifstream ifs( scd_file.c_str() );
  std::string content = "";
  std::string line;
  while ( getline ( ifs,line ) )
  {
    boost::to_lower(line);
    if( line.find("<docid>") == 0 )
    {
      doc_id++;
    }
    else if(line.find("<title>")==0)
    {
      std::string c = line.substr(7, line.length()-7 );
      izenelib::util::UString uc(c, izenelib::util::UString::UTF_8);
      kpe->insert( uc, doc_id);
    }
    else if(line.find("<content>")==0)
    {
      std::string c = line.substr(9, line.length()-9 );
      izenelib::util::UString uc(c, izenelib::util::UString::UTF_8);
      kpe->insert( uc, doc_id);
    }
  }
  
  kpe->close();
  delete kpe;
  delete analyzer;
  boost::filesystem::remove_all(work_dir);
  return 0;

  
}
