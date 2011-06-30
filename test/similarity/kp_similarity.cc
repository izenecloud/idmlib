#include <idmlib/similarity/term_similarity.h>
#include <am/matrix/matrix_mem_io.h>
#include <am/matrix/matrix_file_io.h>
#include <util/ClockTimer.h>
#include <boost/regex.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

int main(int argc, char** argv)
{

  if(argc!=3)
  {
    std::cerr<<"Usage: ./kp_similarity <inverted_text_file> <output_file>"<<std::endl;
    return -1;
  }
  izenelib::util::ClockTimer timer;
  std::string kp_inverted_file(argv[1]);
  std::string output_file(argv[2]);
  std::cout<<"file : "<<kp_inverted_file<<std::endl;
  std::cout<<"output file : "<<output_file<<std::endl;
  std::ifstream ifs(kp_inverted_file.c_str());
  std::string line;
  std::string test_dir = "./term_sim_test";
  std::string rig_dir = "./rig_dir";
  boost::filesystem::remove_all(test_dir);
  uint8_t sim_per_term = 5;
  typedef idmlib::sim::TermSimilarityTable<uint32_t> SimTableType;
  typedef idmlib::sim::SimOutputCollector<SimTableType> SimCollectorType;
  typedef idmlib::sim::TermSimilarity<SimCollectorType> TermSimilarityType;
  SimCollectorType* sim_collector = new SimCollectorType(test_dir+"/sim_collector", sim_per_term);
  
  if(!sim_collector->Open()) 
  {
    std::cerr<<"sim collector open error"<<std::endl;
    return -1;
  }
  TermSimilarityType* sim = new TermSimilarityType(test_dir, rig_dir, sim_collector, 0.4);
  sim->SetDocSim(true);
  if(!sim->Open())
  {
    std::cout<<"open error"<<std::endl;
    return -1;
  }
  uint32_t context_max = 200000;
  if(!sim->SetContextMax(context_max))
  {
    std::cerr<<"SetContextMax error"<<std::endl;
    return -1;
  }
  std::vector<std::string> kp_list;
  uint32_t line_num = 0;
  while( getline(ifs, line) )
  {
    line_num++;
    if(line_num%100==0)
    {
      std::cout<<"Processing line "<<line_num<<std::endl;
    } 
    std::vector<std::string> vec1;
    boost::algorithm::split( vec1, line, boost::algorithm::is_any_of("\t") );
    if(vec1.size()!=2)
    {
      std::cout<<"invalid line: "<<line<<std::endl;
      continue;
    }
    std::string kp = vec1[0];
    std::vector<std::string> vec2;
    boost::algorithm::split( vec2, vec1[1], boost::algorithm::is_any_of("|") );
    std::vector<std::pair<uint32_t, uint32_t> > doc_item_list;
    for(uint32_t i=0;i<vec2.size();i++)
    {
      std::vector<std::string> doc_item;
      boost::algorithm::split( doc_item, vec2[i], boost::algorithm::is_any_of(",") );
      if( doc_item.size()!=2 )
      {
        std::cout<<"invalid doc item: "<<vec2[i]<<std::endl;
        continue;
      }
      uint32_t docid = boost::lexical_cast<uint32_t>(doc_item[0]);
      uint32_t freq = boost::lexical_cast<uint32_t>(doc_item[1]);
      doc_item_list.push_back(std::make_pair(docid, freq));
    }
    if(doc_item_list.empty())
    {
      std::cout<<"invalid line: "<<line<<std::endl;
      continue;
    }
    uint32_t kp_id = kp_list.size()+1;
    //append this kp term
    if(!sim->Append(kp_id, doc_item_list))
    {
      std::cout<<"append error"<<std::endl;
      return -1;
    }
    kp_list.push_back(kp);
  }
  std::cout<<"Start computing similarity"<<std::endl;
  if(!sim->Compute())
  {
    std::cout<<"compute error"<<std::endl;
    return -1;
  }
  std::cout<<"Computing finished."<<std::endl;
  SimTableType* table = sim_collector->GetContainer();
  std::ofstream ofs(output_file.c_str());
  for(uint32_t i=0;i<kp_list.size();i++)
  {
    uint32_t kp_id = i+1;
    std::vector<uint32_t> sim_list;
    if(!table->Get(kp_id, sim_list))
    {
      std::cout<<"no sim list for kp id: "<<kp_id<<std::endl;
    }
    else
    {
      std::string kp = kp_list[kp_id-1];
      ofs<<kp<<" : ";
      for(uint32_t j=0;j<sim_list.size();j++)
      {
        std::string sim_kp = kp_list[sim_list[j]-1];
        ofs<<sim_kp<<",";
      }
      ofs<<std::endl;
    }
  }
  ofs.close();
  std::cout<<"Output finished"<<std::endl;
  std::cout<<"Totally cost "<<timer.elapsed()<<" seconds"<<std::endl;
  delete sim;
  delete table;
  return 0;
}
