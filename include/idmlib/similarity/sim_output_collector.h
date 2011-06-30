#ifndef IDMLIB_SIM_SIMOUTPUTCOLLECTOR_H_
#define IDMLIB_SIM_SIMOUTPUTCOLLECTOR_H_


#include <string>
#include <iostream>
#include <idmlib/idm_types.h>
#include <util/file_object.h>
#include <am/sequence_file/ssfr.h>
NS_IDMLIB_SIM_BEGIN

template <class Container>
class SimOutputCollector
{
public:
  typedef izenelib::am::ssf::Writer<uint8_t> WriterType;
  typedef typename Container::IdType IdType;
  
  SimOutputCollector(const std::string& path, uint32_t max_count, bool print = false)
  :path_(path), container_path_(path+"/container"), writer_path_(path+"/writer")
  , max_count_(max_count), print_(print), writer_(NULL), max_id_(0)
  {
  }
  
  ~SimOutputCollector()
  {
    if(container_!=NULL)
    {
        delete container_;
    }
    if(writer_!=NULL)
    {
      delete writer_;
      writer_ = NULL;
    }
  }
  
  bool Open()
  {
      try{
          boost::filesystem::create_directories(path_);
      }
      catch(std::exception& ex)
      {
          std::cerr<<ex.what()<<std::endl;
          return false;
      }
      container_ = new Container(container_path_);
      if(!container_->Open())
      {
          std::cerr<<"sim output collector container open failed"<<std::endl;
      }
      return Init_();
    
  }
  
  
  
  bool Flush()
  {
    if(writer_==NULL) return true;
    writer_->Close();
    delete writer_;
    writer_ = NULL;
    container_->ResizeIf(max_id_);
    izenelib::am::ssf::Sorter<uint8_t, IdType, false>::Sort(writer_path_);
    izenelib::am::ssf::Reader<uint8_t> reader(writer_path_);
    if(!reader.Open()) return false;
    IdType base = 0;
    std::pair<IdType, double> sim;
    IdType last_base = 0;
    std::vector<std::pair<double, IdType> > sim_list;
    while(reader.Next(base, sim))
    {
      if(base!=last_base && !sim_list.empty())
      {
        std::sort(sim_list.begin(), sim_list.end(), std::greater<std::pair<double, IdType> >());
        if(!UpdateSim_(last_base, sim_list))
        {
          return false;
        }
        sim_list.resize(0);
      }
      sim_list.push_back(std::make_pair(sim.second, sim.first));
      last_base = base;
    }
    if(!sim_list.empty())
    {
      std::sort(sim_list.begin(), sim_list.end(), std::greater<std::pair<double, IdType> >());
      if(!UpdateSim_(last_base, sim_list))
      {
        return false;
      }
      sim_list.resize(0);
    }
    reader.Close();
    if(!container_->Flush())
    {
      std::cerr<<"sim table flush failed"<<std::endl;
      return false;
    }
    return Init_();
  }
  
  bool AddSimPair(IdType a, IdType b, double score)
  {
    if(!Add_(a,b,score)) return false;
    if(!Add_(b,a,score)) return false;
    return true;
  }
  
  Container* GetContainer() const
  {
      return container_;
  }
 
 private:
  bool Init_()
  {
    try
    {
      max_id_ = 0;
      boost::filesystem::remove_all(writer_path_);
      writer_ = new WriterType(writer_path_);
      if(!writer_->Open()) return false;
    }
    catch(std::exception& ex)
    {
      std::cerr<<ex.what()<<std::endl;
      return false;
    }
    return true;
  }
  
  bool Add_(IdType base, IdType sim, double score)
  {
    if(base>max_id_) max_id_ = base;
    if(!writer_->Append(base, std::make_pair(sim, score) ))
    {
      std::cerr<<"sim collector add_ "<<base<<","<<sim<<","<<score<<" failed"<<std::endl;
      return false;
    }
    return true;
  }
  
  bool UpdateSim_(IdType base, const std::vector<std::pair<double, IdType> >& sim_list)
  {
    uint32_t count = sim_list.size()>max_count_?max_count_:sim_list.size();
    std::vector<IdType> sim_vec;
    for(uint32_t c=0;c<count;c++)
    {
      
      sim_vec.push_back(sim_list[c].second);
      
    }
    //storage the similar items
    if(!container_->Update(base, sim_vec))
    {
      std::cerr<<"sim table update on "<<base<<" failed."<<std::endl;
      return false;
    }
    if(print_)
    {
      std::cout<<"[SIM] ["<<base<<"] ";
      for(uint32_t i=0;i<sim_vec.size();i++)
      {
        std::cout<<"("<<sim_list[i].second<<","<<sim_list[i].first<<") ";
      }
      std::cout<<std::endl;
    }
    {
      //debug score
//         std::cout<<"[score] "<<i<<" : "<<for_sort[0].first<<","<<for_sort[count-1].first<<std::endl;
    }
    return true;
  }

 private: 
  Container* container_;
  std::string path_;
  std::string container_path_;
  std::string writer_path_;
  uint32_t max_count_;
  bool print_;
  WriterType* writer_;
  IdType max_id_;
};

   
NS_IDMLIB_SIM_END



#endif 
