#ifndef IDMLIB_SIM_SIMOUTPUTHANDLER_H_
#define IDMLIB_SIM_SIMOUTPUTHANDLER_H_


#include <string>
#include <iostream>
#include <idmlib/idm_types.h>
#include <util/file_object.h>

NS_IDMLIB_SIM_BEGIN

template <class Container>
class SimOutputCollector
{
public:
  typedef izenelib::am::ssf::Writer<uint8_t> WriterType;
  typedef typename Container::IdType IdType;
  
  SimOutputCollector(Container* container, const std::string& path, uint32_t max_count, bool print = false)
  :container_(container), path_(path), max_count_(max_count), print_(print), writer_(NULL), max_id_(0)
  {
  }
  
  ~SimOutputCollector()
  {
    if(writer_!=NULL)
    {
      delete writer_;
      writer_ = NULL;
    }
  }
  
  bool Open()
  {
    return Init_();
    
  }
  
  bool Flush()
  {
    if(writer_==NULL) return true;
    writer_->Close();
    delete writer_;
    writer_ = NULL;
    container_->ResizeIf(max_id_);
    izenelib::am::ssf::Sorter<uint8_t, IdType, false>::Sort(path_);
    izenelib::am::ssf::Reader<uint8_t> reader(path_);
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
  
  bool Add(IdType a, IdType b, double score)
  {
    if(!Add_(a,b,score)) return false;
    if(!Add_(b,a,score)) return false;
    return true;
  }
  
 
 private:
  bool Init_()
  {
    try
    {
      max_id_ = 0;
      writer_ = new WriterType(path_);
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
  uint32_t max_count_;
  bool print_;
  WriterType* writer_;
  IdType max_id_;
};

   
NS_IDMLIB_SIM_END



#endif 
