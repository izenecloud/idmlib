#include <idmlib/keyphrase-extraction/kpe_algorithm.h>
#include <idmlib/nec/NameEntity.h>
// #include <idmlib/nec/NameEntityUtil.h>
#include <idmlib/nec/NameEntityDict.h>
#include <idmlib/nec/NameEntityManager.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <util/scd_parser.h>
#include <util/ustring/UString.h>
#include <boost/algorithm/string.hpp>
#include <am/3rdparty/rde_hash.h>
#include <boost/lexical_cast.hpp>
using namespace idmlib;
using namespace idmlib::kpe;
using namespace idmlib::util;
using namespace boost::filesystem;
namespace po = boost::program_options;


class KPEResult
{
  typedef std::pair<uint32_t, uint32_t> id2count_t;
  typedef std::pair<izenelib::util::UString, uint32_t> str2count_t;
  public:
    izenelib::util::UString str;
    std::vector<id2count_t> id2countList;
    std::vector<str2count_t> leftTermList;
    std::vector<str2count_t> rightTermList;
     
  public:
    void get_all_feature_values(std::vector<std::pair<std::string, double> >& features)
    {
      izenelib::util::UString surface = str;
      features.resize(0);
      if(surface.length()<1) return;
      uint32_t freq = 0;
      for(uint32_t i=0;i<id2countList.size();i++)
      {
        freq += id2countList[i].second;
      }
      if(freq<1) return;
      izenelib::util::UString u_pu = surface.substr(0,1);
      std::string pu;
      u_pu.convertString(pu, izenelib::util::UString::UTF_8);
      std::pair<std::string, double> f_pu("PU|"+pu, 1.0);
      features.push_back(f_pu);
      izenelib::util::UString u_su = surface.substr(surface.length()-1,1);
      std::string su;
      u_su.convertString(su, izenelib::util::UString::UTF_8);
      std::pair<std::string, double> f_su("SU|"+su, 1.0);
      features.push_back(f_su);
      if(surface.length()>1)
      {
        izenelib::util::UString u_pb = surface.substr(0,2);
        std::string pb;
        u_pb.convertString(pb, izenelib::util::UString::UTF_8);
        std::pair<std::string, double> f_pb("PB|"+pb, 1.0);
        features.push_back(f_pb);
        izenelib::util::UString u_sb = surface.substr(surface.length()-2,2);
        std::string sb;
        u_sb.convertString(sb, izenelib::util::UString::UTF_8);
        std::pair<std::string, double> f_sb("SB|"+sb, 1.0);
        features.push_back(f_sb);
      }
      
      izenelib::am::rde_hash<std::string, bool> apps;
      for(uint32_t i=0;i<leftTermList.size();i++)
      {
        double value = (double)(leftTermList[i].second)/freq;
        std::string str_context;
        leftTermList[i].first.convertString(str_context, izenelib::util::UString::UTF_8);
        if(str_context.length()==0) continue;
        std::pair<std::string, double> f_context("LC|"+str_context, value);
        features.push_back(f_context);
        
      }
      
      for(uint32_t i=0;i<rightTermList.size();i++)
      {
        double value = (double)(rightTermList[i].second)/freq;
        std::string str_context;
        rightTermList[i].first.convertString(str_context, izenelib::util::UString::UTF_8);
        if(str_context.length()==0) continue;
        std::pair<std::string, double> f_context("RC|"+str_context, value);
        features.push_back(f_context);
      }
      
    }
};

class NERAccuracier
{
  typedef std::pair<uint32_t, uint32_t> id2count_t;
  typedef std::pair<izenelib::util::UString, uint32_t> str2count_t;
 public:
  NERAccuracier(const std::string& nec_path, idmlib::util::IDMIdManager* id_manager, uint32_t test_count, const std::vector<std::pair<izenelib::util::UString, int> >& ner_type_list)
  :id_manager_(id_manager), test_count_(test_count)
  {
    for(uint32_t i=0;i<ner_type_list.size();i++)
    {
      ner_types_.insert(ner_type_list[i].first, ner_type_list[i].second);
    }
    idmlib::NameEntityManager& nec = idmlib::NameEntityManager::getInstance(nec_path);
    nec_ = &nec;
    
  }
  
  ~NERAccuracier()
  {
  }
  
  void Callback(
    const izenelib::util::UString& str
  , const std::vector<id2count_t>& id2countList
  , uint8_t score
  , const std::vector<id2count_t>& leftTermList
  , const std::vector<id2count_t>& rightTermList)
  {
    bool valid4test = false;
    for(uint32_t i=0;i<id2countList.size();i++)
    {
      uint32_t docid = id2countList[i].first;
      if(docid<=test_count_)
      {
        valid4test = true;
        break;
      }
    }
    if(!valid4test) return;
    KPEResult kpe_result;
    kpe_result.str = str;
    kpe_result.id2countList = id2countList;
    
    std::vector<str2count_t> leftTermStrList(leftTermList.size());
    std::vector<str2count_t> rightTermStrList(rightTermList.size());
    std::vector<izenelib::util::UString> prefixList;
    for(uint32_t i=0;i<leftTermList.size();i++)
    {
        izenelib::util::UString prefix;
        if( id_manager_->GetStringById(leftTermList[i].first, prefix) )
        {
            prefixList.push_back(prefix);
            leftTermStrList[i].first = prefix;
            leftTermStrList[i].second = leftTermList[i].second;
        }
    }
    
    std::vector<izenelib::util::UString> suffixList;
    for(uint32_t i=0;i<rightTermList.size();i++)
    {
        izenelib::util::UString suffix;
        if( id_manager_->GetStringById(rightTermList[i].first, suffix) )
        {
            suffixList.push_back(suffix);
            rightTermStrList[i].first = suffix;
            rightTermStrList[i].second = rightTermList[i].second;
        }
    }
    kpe_result.leftTermList = leftTermStrList;
    kpe_result.rightTermList = rightTermStrList;
    idmlib::NameEntity ne(str, prefixList, suffixList);
    
    nec_->predict(ne);
    std::vector<ml::Label>& mllabels= ne.predictLabels;
    int type = 0;
    for(uint32_t i=0;i<mllabels.size();i++)
    {
        if(mllabels[i] == "PEOP" )
        {
            type = 1;
            break;
        }
        else if( mllabels[i] == "LOC" )
        {
            type = 2;
            break;
        }
        else if( mllabels[i] == "ORG" )
        {
            type = 3;
            break;
        }
    }
    test_result_.push_back(std::make_pair(kpe_result, type));
  //         std::string ss;
  //         str.convertString(ss, izenelib::util::UString::UTF_8);
  //         std::cout<<"label type - "<<ss<<" : "<<(uint32_t)labelType<<std::endl;
      
      
  }
  
  void print_stat()
  {
    int total = 0;
    int correct = 0;
    for(uint32_t i=0;i<test_result_.size();i++)
    {
      int type = test_result_[i].second;
      std::string str;
      test_result_[i].first.str.convertString(str, izenelib::util::UString::UTF_8);
      std::cout<<str<<","<<type<<std::endl;
      if(type>0)
      {
        total++;
        int* o = ner_types_.find(test_result_[i].first.str);
        if(o!=NULL)
        {
          std::cout<<"[["<<str<<","<<(*o)<<std::endl;
          if((*o)==type)
          {
            correct++;
          }
        }
      }
      
    }
    std::cout<<"accuracy ("<<correct<<","<<total<<") : "<<(double)correct/total<<std::endl;
  }
  
  void output_libsvm(const std::string& index_file, const std::string& output)
  {
    izenelib::am::rde_hash<std::string, int> feature_index;
    {
      std::ifstream ifs(index_file.c_str());
      std::string line;
      int index = 1;
      while(getline(ifs, line))
      {
        feature_index.insert(line, index);
        index++;
      }
      ifs.close();
    }
    std::string libsvm_output = output+".libsvm";
    std::ofstream ofs(libsvm_output.c_str());
    std::string info_output = output+".info";
    std::ofstream info_ofs(info_output.c_str());
    std::vector<std::pair<std::string, double> > features;
    for(uint32_t i=0;i<test_result_.size();i++)
    {
      int type = test_result_[i].second;
      izenelib::util::UString surface = test_result_[i].first.str;
      test_result_[i].first.get_all_feature_values(features);
      std::string str;
      surface.convertString(str, izenelib::util::UString::UTF_8);
      if(features.size()==0) continue;
      std::vector<std::pair<int, double> > feature_list;
      for(uint32_t i=0;i<features.size();i++)
      {
        std::string label = features[i].first;
        std::cout<<"["<<str<<"] find label : "<<label<<std::endl;
        int* index = feature_index.find(label);
        if(index!=NULL)
        {
          feature_list.push_back(std::make_pair(*index, features[i].second));
        }
      }
      std::sort(feature_list.begin(), feature_list.end());
      ofs<<0;
      for(uint32_t i=0;i<feature_list.size();i++)
      {
        ofs<<" "<<feature_list[i].first<<":"<<feature_list[i].second;
      }
      ofs<<std::endl;
      info_ofs<<type<<"\t"<<str<<std::endl;
    }
    ofs.close();
    info_ofs.close();
  }
  
 private:
  idmlib::NameEntityManager* nec_;
  idmlib::util::IDMIdManager* id_manager_;
  uint32_t test_count_;
  izenelib::am::rde_hash<izenelib::util::UString, int> ner_types_;
  
  std::vector<std::pair<KPEResult, int> > test_result_;
};

int main(int ac, char** av)
{
  po::options_description desc("Allowed options");
  desc.add_options()
    ("help", "produce help message")
    ("resource-path,R", po::value<std::string>(), "resource path include kpe and ner")
    ("scd-file,S", po::value<std::string>(), "scd file to be processed")
    ("property,P", po::value<std::string>(), "properties to be processed, seperate with comma(Title,Content), case insensitive")
    ("input-file,I", po::value<std::string>(), "input human defined named entities used for test the accuracy")
    ("feature-index,F", po::value<std::string>(), "feature index file used for generating libsvm file")
    ("kma-path,K", po::value<std::string>(), "if we want to process Korean collection, specify this kma path")
    ("working-path,W", po::value<std::string>(), "temp working path used for kpe, default: ./kpe_scd_working")
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
  
  std::string kpe_resource_path = resource_path+"/kpe";
  std::string ner_resource_path = resource_path+"/nec";
 
  
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
  
  std::string feature_index_file;
  if (vm.count("feature-index")) {
    feature_index_file = vm["feature-index"].as<std::string>();
    std::cout << "feature-index: " << feature_index_file <<std::endl;
  } 
  else {
    std::cout<<"feature-index not set"<<std::endl;
  }
  
  std::string input_file;
  if (vm.count("input-file")) {
    input_file = vm["input-file"].as<std::string>();
    std::cout << "input-file: " << input_file <<std::endl;
  } 
  else {
    std::cerr<<"input-file not set"<<std::endl;
    std::cerr << desc << std::endl;
    return -1;
  }
  uint32_t num4test = 0;
  uint32_t max_doc = 0;
  izenelib::am::rde_hash<izenelib::util::UString, int> ner_types;
  std::vector<std::pair<izenelib::util::UString, int> > ner_type_list;
  {
    std::ifstream ifs(input_file.c_str());
    std::string line;
    getline ( ifs,line );
    std::vector<std::string> vec_value;
    boost::algorithm::split( vec_value, line, boost::algorithm::is_any_of(",") );
    num4test = boost::lexical_cast<uint32_t>(vec_value[0]);
    max_doc = boost::lexical_cast<uint32_t>(vec_value[1]);
    while ( getline ( ifs,line ) )
    {
      std::vector<std::string> vec;
      boost::algorithm::split( vec, line, boost::algorithm::is_any_of("\t") );
      izenelib::util::UString term(vec[0], izenelib::util::UString::UTF_8);
      int type = boost::lexical_cast<int>(vec[1]);
      ner_type_list.push_back(std::make_pair(term, type));
//       std::cout<<vec_value[1]<<","<<type<<std::endl;
    }
    ifs.close();
  }
  
  std::string scd_file;
  if (vm.count("scd-file")) {
    scd_file = vm["scd-file"].as<std::string>();
    std::cout << "scd-file: " << scd_file <<std::endl;
  } 
  else {
    std::cerr<<"scd-file not set"<<std::endl;
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
  
  std::string id_manager_path = working_path+"/id-manager/";
  boost::filesystem::create_directories(id_manager_path);
  idmlib::util::IDMIdManager* id_manager_ = new idmlib::util::IDMIdManager(id_manager_path);
  
  typedef KPEOutput<true, true, true> OutputType ;
  typedef OutputType::function_type function_type ;
  if( !analyzer->LoadT2SMapFile(kpe_resource_path+"/cs_ct") )
  {
    return -1;
  }
  
  NERAccuracier ner_accuracy(ner_resource_path, id_manager_, num4test, ner_type_list);
  function_type callback_func = boost::bind( &NERAccuracier::Callback, &ner_accuracy, _1, _2, _3, _4, _5);
  KPEAlgorithm<OutputType>* kpe = new KPEAlgorithm<OutputType>(working_path, analyzer, callback_func, id_manager_);
  if( !kpe->load(kpe_resource_path) )
  {
    return -1;
  }
  kpe->set_no_freq_limit();
  for(uint32_t i=0;i<ner_type_list.size();i++)
  {
    std::string str;
    ner_type_list[i].first.convertString(str, izenelib::util::UString::UTF_8);
    std::cout<<"add manmade : "<<str<<","<<ner_type_list[i].second<<std::endl;
    kpe->add_manmade(ner_type_list[i].first);
  }
  izenelib::util::UString tracing("佛山", izenelib::util::UString::UTF_8);
  kpe->set_tracing(tracing);
  uint32_t docid = 0;
  
  izenelib::util::ScdParser scd_parser(encoding);
  if(!scd_parser.load(scd_file) )
  {
    std::cerr<<"load scd file failed."<<std::endl;
    return -1;
  }
  izenelib::util::ScdParser::iterator it = scd_parser.begin();
  bool end = false;
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
        if( max_doc>0 && docid > max_doc ) 
        {
          end = true;
          break;
        }
        if( docid % 1000 == 0 )
        {
          std::cout<<"Processing "<<docid<<std::endl;
        }
      }
      else if( properties.find( property_name) != NULL)
      {
//         std::string str;
//         p->second.convertString(str, izenelib::util::UString::UTF_8);
//         std::cout<<str<<std::endl;
        kpe->insert( p->second, docid);
      }
    }
    if(end) break;
    ++it;
  }
  
  kpe->close();
    
  delete kpe;
  delete analyzer;
  delete id_manager_;
  boost::filesystem::remove_all(working_path);
  ner_accuracy.print_stat();
  if(feature_index_file!="")
  {
    time_t _time = time(NULL);
    struct tm * timeinfo;
    char buffer[14];
    timeinfo = localtime(&_time);
    strftime(buffer, 80, "%Y%m%d%H%M%S", timeinfo);
    std::string time_stamp = buffer;
    ner_accuracy.output_libsvm(feature_index_file, "./libsvm_output_"+time_stamp);
  }
  return 0;

  
}
