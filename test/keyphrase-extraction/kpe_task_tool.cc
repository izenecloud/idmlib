#include <idmlib/keyphrase-extraction/kpe_task.h>
#include <idmlib/keyphrase-extraction/kpe_evaluate.h>
#include <boost/algorithm/string.hpp>
#include <boost/unordered_map.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <util/scd_parser.h>
#include <util/functional.h>
#include "../TestResources.h"
using namespace idmlib;
using namespace idmlib::kpe;
using namespace idmlib::util;
using namespace boost::filesystem;
namespace po = boost::program_options;

typedef izenelib::util::UString StringType;

// class Callback
// {
//     public:
//         Callback(const std::string& kp_file, const std::string& doc_kp_file)
//         :kp_ofs_( (kp_file).c_str()), title_ofs_( (kp_file+".title").c_str()), first_(true), t2k_ofs_( (doc_kp_file+".t2k").c_str()), k2t_ofs_( (doc_kp_file+".k2t").c_str())
//         {
//         }
//         
//        
//         void FindKp(uint32_t kpid, const izenelib::util::UString& text, uint32_t freq)
//         {
//             std::string str;
//             text.convertString(str, izenelib::util::UString::UTF_8);
//             if(str.empty())
//             {
//                 first_ = false;
//                 return;
//             }
//             boost::algorithm::replace_all(str, " ", "_");
//             boost::algorithm::replace_all(str, "\r", "_");
//             boost::algorithm::replace_all(str, "\n", "_");
//             if(first_)
//             {
//                 kp_ofs_<<kpid<<" "<<str<<" "<<freq<<std::endl;
//             }
//             else
//             {
//                 title_ofs_<<kpid<<" "<<str<<" "<<freq<<std::endl;
//             }
//         }
//         
//         void FindDoc(uint32_t docid, const std::vector<uint32_t>& id_list)
//         {
//             std::vector<uint32_t> title_id_list;
//             std::vector<uint32_t> kp_id_list;
//             uint32_t i=0;
//             bool first = true;
//             while(i<id_list.size())
//             {
//                 if(id_list[i]==0)
//                 {
//                     first = false;
//                 }
//                 else
//                 {
//                     if(!first)
//                     {
//                         kp_id_list.push_back(id_list[i]);
//                     }
//                     else
//                     {
//                         title_id_list.push_back(id_list[i]);
//                     }
//                 }
//                 ++i;
//             }
//             
//             {
//                 t2k_ofs_<<"1"<<std::endl;
//                 for(i=0;i<title_id_list.size();i++)
//                 {
//                     if(i>0) t2k_ofs_<<" ";
//                     t2k_ofs_<<title_id_list[i];
//                 }
//                 t2k_ofs_<<std::endl;
//                 for(i=0;i<kp_id_list.size();i++)
//                 {
//                     if(i>0) t2k_ofs_<<" ";
//                     t2k_ofs_<<kp_id_list[i];
//                 }
//                 t2k_ofs_<<std::endl;
//             }
//             
//             {
//                 k2t_ofs_<<"1"<<std::endl;
//                 for(i=0;i<kp_id_list.size();i++)
//                 {
//                     if(i>0) k2t_ofs_<<" ";
//                     k2t_ofs_<<kp_id_list[i];
//                 }
//                 k2t_ofs_<<std::endl;
//                 for(i=0;i<title_id_list.size();i++)
//                 {
//                     if(i>0) k2t_ofs_<<" ";
//                     k2t_ofs_<<title_id_list[i];
//                 }
//                 k2t_ofs_<<std::endl;
//             }
//         }
//         
//         void Close()
//         {
//             kp_ofs_.close();
//             title_ofs_.close();
//             t2k_ofs_.close();
//             k2t_ofs_.close();
//         }
//         
//     private:
//         std::ofstream kp_ofs_;
//         std::ofstream title_ofs_;
//         bool first_;
//         std::ofstream t2k_ofs_;
//         std::ofstream k2t_ofs_;
// };


struct ScoreItem
{
    ScoreItem(): tf(0.0), df(0.0)
    {
    }
    double tf;
    double df;
};

class Callback2
{
    public:
        Callback2(const std::string& result_file, bool chinese_only = false)
        :ofs_(result_file.c_str()), adf_(0), chinese_only_(chinese_only)
        {
            
        }
        
        void FindDoc(uint32_t docid, const std::vector<DocKpItem>& kp_list)
        {
            std::string str_docid = "UNKNOW";
            boost::unordered_map<uint32_t, std::string>::iterator it = docid_map_.find(docid);
            if(it!=docid_map_.end())
            {
                str_docid = it->second;
            }
            ofs_<<str_docid<<std::endl;
            for(uint32_t i=0;i<kp_list.size();i++)
            {
                if(kp_list[i].score==0.0)
                {
                    //std::cout<<"SCORE 0 : "<<str<<std::endl;
                    continue;
                }
                bool has_chinese = false;
                for(uint32_t c=0;c<kp_list[i].text.length();c++)
                {
                    if(kp_list[i].text.isChineseChar(c))
                    {
                        has_chinese = true;
                        break;
                    }
                }
                if(!chinese_only_ || has_chinese)
                {
                    std::string str;
                    kp_list[i].text.convertString(str, StringType::UTF_8);
                    ofs_<<str<<",";
                    kp_map_[str].tf += kp_list[i].score;
                    kp_map_[str].df += 1;
                }
            }
            ofs_<<std::endl;
            adf_+=1;
        }
        
        void SetDocId(uint32_t docid, const std::string& str_docid)
        {
            docid_map_.insert(std::make_pair(docid, str_docid));
        }
        
        void Close()
        {
            std::vector<std::pair<std::string, double> > kp_list;
            for(boost::unordered_map<std::string, ScoreItem>::const_iterator it = kp_map_.begin(); it!=kp_map_.end(); ++it)
            {
                double tfidf = std::log( (double)adf_/it->second.df) * it->second.tf;
                kp_list.push_back(std::make_pair(it->first, tfidf));
            }
            typedef izenelib::util::second_greater<std::pair<std::string, double> > greater_than;
            std::sort(kp_list.begin(), kp_list.end(), greater_than());
            ofs_<<"=========KP BEGIN========"<<std::endl;
            for(uint32_t i=0;i<kp_list.size();i++)
            {
                ofs_<<kp_list[i].first<<","<<kp_list[i].second<<std::endl;
            }
            ofs_.close();
        }
        
    private:
        std::ofstream ofs_;
        boost::unordered_map<uint32_t, std::string> docid_map_;
        boost::unordered_map<std::string, ScoreItem > kp_map_;
        uint32_t adf_;
        bool chinese_only_;
        
};

int main(int ac, char** av)
{
  po::options_description desc("Allowed options");
  desc.add_options()
    ("help", "produce help message")
    ("resource-path,R", po::value<std::string>(), "KPE resource path")
    ("title-property,T", po::value<std::string>(), "title property name")    
    ("content-property,C", po::value<std::string>(), "content property name")    
    ("scd-path,S", po::value<std::string>(), "scd directory to be processed")
    ("output-file,O", po::value<std::string>(), "output key-phrases file for each doc")
    ("working-path,W", po::value<std::string>(), "temp working path used for kpe, default: ./kpe_scd_working")
    ("max-doc,M", po::value<uint32_t>(), "max doc count which will be processed.")
    ("chinese-only", "only output KPs which have at least chinese character.")
  ;
  std::string default_working_path = "./kpe_task_working";
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
          std::string file_name = itr->path().filename().string();
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
  
  
  
  idmlib::util::IDMAnalyzer* analyzer = new idmlib::util::IDMAnalyzer(idmlib::util::IDMAnalyzerConfig::GetCommonTgConfig("","",""));
  idmlib::util::IDMAnalyzer* cma_analyzer = new idmlib::util::IDMAnalyzer(idmlib::util::IDMAnalyzerConfig::GetCommonConfig("",IZENECMA_KNOWLEDGE,""));

  
  std::string working_path;
  if (vm.count("working-path")) {
    working_path = vm["working-path"].as<std::string>();
  } 
  else {
    std::cout<<"working-path not set, use default "<<default_working_path<<std::endl;
    working_path = default_working_path;
  }
  
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
  
  uint32_t max_doc = 0;
  if (vm.count("max-doc")) {
    max_doc = vm["max-doc"].as<uint32_t>();
    std::cout << "max-doc: " << max_doc <<std::endl;
  } 
  
  bool chinese_only = false;
  if(vm.count("chinese-only"))
  {
      chinese_only = true;
      std::cout<<"set chinese-only"<<std::endl;
  }
  
  if( !analyzer->LoadT2SMapFile(resource_path+"/cs_ct") )
  {
    return -1;
  }
  std::string docid_property("docid");
  std::string title_property("title");
  std::string content_property("content");
  if (vm.count("title-property")) {
    title_property = vm["title-property"].as<std::string>();
  } 
  if (vm.count("content-property")) {
    content_property = vm["content-property"].as<std::string>();
  } 
  boost::to_lower(docid_property);
  boost::to_lower(title_property);
  boost::to_lower(content_property);  
  idmlib::util::IDMIdManager* id_manager = new idmlib::util::IDMIdManager(working_path+"/idmanager");
  KpeKnowledge* knowledge = new KpeKnowledge(analyzer);
  knowledge->Load(resource_path);
  KpeTask task(working_path, analyzer, cma_analyzer, id_manager, knowledge);
  
  Callback2 callback(output_file, chinese_only);
  
//   KpeTask::KpTextCallback kp_callback = boost::bind( &Callback::FindKp, &callback, _1, _2, _3);
  KpeTask::DocKpCallback doc_callback = boost::bind( &Callback2::FindDoc, &callback, _1, _2);
  task.SetCallback(doc_callback);
  
  uint32_t docid = 0;
  bool limited = false;
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
      izenelib::util::UString title;
      izenelib::util::UString content;
      std::vector<std::pair<izenelib::util::UString, izenelib::util::UString> >::iterator p;
      for (p = doc->begin(); p != doc->end(); p++)
      {
        izenelib::util::UString property_name = p->first;
        property_name.toLowerString();
        std::string str_property;
        property_name.convertString(str_property, izenelib::util::UString::UTF_8);
        if( str_property == docid_property )
        {
          docid++;
          if( max_doc>0 && docid > max_doc ) 
          {
              limited = true;
              break;
          }
          if( docid % 1000 == 0 )
          {
            std::cout<<"Processing "<<docid<<std::endl;
          }
          std::string str_docid;
          p->second.convertString(str_docid, StringType::UTF_8);
          callback.SetDocId(docid, str_docid);
        }
        else if( str_property == title_property )
        {
            title = p->second;
        }
        else if( str_property == content_property)
        {
            content = p->second;
        }
      }
      if(limited) break;
      if(title.length()>0 && content.length()>0)
      {
        task.Insert(docid, title, content);
      }
      ++it;
    }
    if(limited) break;
  }
  std::cout<<"max docid "<<docid<<std::endl;
  
  task.Close();
  callback.Close();
  delete analyzer;
  delete cma_analyzer;
  delete knowledge;
  delete id_manager;
  boost::filesystem::remove_all(working_path);
  
  return 0;

  
}
