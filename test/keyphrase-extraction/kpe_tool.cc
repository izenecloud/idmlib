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
  std::string file_dir(av[2]);
  typedef KPEOutput<true, false, false> OutputType ;
  IDMAnalyzer* analyzer = new IDMAnalyzer(idmlib::util::IDMAnalyzerConfig::GetCommonConfig(WISEKMA_KNOWLEDGE,"",""));
  analyzer->LoadT2SMapFile(resource_path+"/cs_ct");
  std::string work_dir = "./kpe_working";
  boost::filesystem::remove_all(work_dir);
  KPEAlgorithm<OutputType>* kpe = new KPEAlgorithm<OutputType>(work_dir, analyzer, &Callback);
  kpe->load(resource_path);
  uint32_t doc_id = 1;
  directory_iterator end_itr; // default construction yields past-the-end
  for ( directory_iterator itr( file_dir );
        itr != end_itr;
        ++itr )
  {
    if ( is_regular(itr->status()) )
    {
      std::ifstream ifs( itr->path().file_string().c_str() );
      std::string content = "";
      std::string line;
      while ( getline ( ifs,line ) )
      {
        content += line+"\n";
      }
      izenelib::util::UString ucontent(content, izenelib::util::UString::UTF_8);
      kpe->insert( ucontent, doc_id);
      doc_id++;
    }
  }
  kpe->close();
  delete kpe;
  delete analyzer;
  boost::filesystem::remove_all(work_dir);
  return 0;

  
}
