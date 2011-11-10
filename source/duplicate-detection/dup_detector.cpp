#include <idmlib/duplicate-detection/dup_detector.h>
#include <idmlib/duplicate-detection/dd_constants.h>
#include <util/CBitArray.h>
#include <util/ClockTimer.h>
#include <util/url_util.h>
#include <utility>

#include <boost/lexical_cast.hpp>
#include <idmlib/similarity/term_similarity.h>

// #define DUPD_GROUP_DEBUG;
// #define DUPD_TEXT_DEBUG;
// #define DUPD_DEBUG;
// #define DUPD_FP_DEBUG;
// #define NO_IN_SITE;
using namespace std;
using namespace idmlib::dd;

DupDetector::DupDetector(const std::string& container, GroupTable* group_table)
:container_(container)
, maxk_(0), partition_num_(0)
, algo_(NULL)
, writer_(NULL)
, group_table_(group_table)
{
  SetParameters_();
}

void DupDetector::SetParameters_()
{
  maxk_ = 6;
  partition_num_ = 9;
  trash_threshold_ = 0.02;
  trash_min_ = 200;
}

uint8_t DupDetector::GetK_(uint32_t doc_length)
{
  // (document length, k) follows a log based regression function
  // while the document length gets larger, k value decreases.
  // 1 is added to doc_length because ln(0) is not defined (1 is a kind of smothing factor)
//   return 3;
  double y = -1.619429845 * log((double)(doc_length + 1)) + 17.57504447;
  if(y <= 0) y = 0;
  uint8_t k = (uint8_t)y;
  if(y-(double)k>=0.5) k++;
  if(k > 6) k = 6;

  return k;
}

DupDetector::~DupDetector() {
  if(algo_)
    delete algo_;
}

bool DupDetector::Open()
{
    fp_storage_path_ = container_+"/fp";
    fp_working_path_ = container_+"/fp_working";
    try
    {
        boost::filesystem::create_directories(container_);
        boost::filesystem::remove_all(fp_working_path_);
    }
    catch(std::exception& ex)
    {
        std::cerr<<ex.what()<<std::endl;
        return false;
    }

    algo_ = new CharikarAlgo(container_.c_str(), DdConstants::f, 0.95);
    FpTables tables;
    tables.GenTables(DdConstants::f, maxk_, partition_num_, table_list_);
    std::cout<<"Generated "<<table_list_.size()<<" tables for maxk="<<(int)maxk_<<std::endl;

//     std::string inject_file = container_+"/inject.txt";
//     std::string group_file = container_+"/group";
// 
//     if(id_manager_ && boost::filesystem::exists(inject_file))
//     {
//         std::cout<<"start inject data to DD result"<<std::endl;
//         boost::filesystem::remove_all(group_file);
//         group_ = new GroupType( group_file );
//         if(!group_->Load())
//         {
//             std::cerr<<"Load group info failed"<<std::endl;
//         }
//         std::ifstream ifs(inject_file.c_str());
//         std::string line;
//         std::vector<uint32_t> in_group;
//         while ( getline ( ifs,line ) )
//         {
//             boost::algorithm::trim(line);
//             if(line.length()==0 && in_group.size()>0)
//             {
//                 //process in_group
//                 for(uint32_t i=1;i<in_group.size();i++)
//                 {
//                     group_->AddDoc(in_group[0], in_group[i]);
//                 }
//                 
//                 in_group.resize(0);
//                 continue;
//             }
//             if(line.length()>0)
//             {
//                 std::vector<std::string> vec;
//                 boost::algorithm::split( vec, line, boost::algorithm::is_any_of("\t") );
//     //                 std::cout<<"XXX "<<vec[0]<<std::endl;
//                 izenelib::util::UString str_docid(vec[0], izenelib::util::UString::UTF_8);
//                 
//                 uint32_t docid = 0;
//                 if(id_manager_->getDocIdByDocName(str_docid, docid,false) )
//                 {
//                     in_group.push_back(docid);
//                 }
//                 else
//                 {
//                     std::cout<<"docid "<<vec[0]<<" does not exists."<<std::endl;
//                 }
//                 
//             }
//             
//         }
//         ifs.close();
//         //process in_group
//         if(in_group.size()>0)
//         {
//             for(uint32_t i=1;i<in_group.size();i++)
//             {
//                 group_->AddDoc(in_group[0], in_group[i]);
//             }
//         }
//         group_->Flush();
//         boost::filesystem::remove_all(inject_file);
//         std::cout<<"updated dd by "<<inject_file<<std::endl;
//     //         boost::filesystem::remove_all(output_group_file);
//     //         std::string reoutput_file = container_+"/re_output_group.txt";
//     //         OutputResult_(reoutput_file);
//     }
//     else
//     {
//         group_ = new GroupType( group_file );
//         if(!group_->Load())
//         {
//             std::cerr<<"Load group info failed"<<std::endl;
//         }
//     }
//     std::string continue_file = container_+"/continue";
//     if(boost::filesystem::exists(continue_file))
//     {
//       boost::filesystem::remove_all(continue_file);
//       runDuplicateDetectionAnalysis(true);
//     }
    return true;

}

bool DupDetector::SkipTrash_(uint32_t total_count, uint32_t pairwise_count, uint32_t table_id)
{
  if(table_id==0) return false; //first table.
  if(pairwise_count<trash_min_) return false;
  else return true;
//   double d = (double)pairwise_count/total_count;
//   if(d>=trash_threshold_)
//   {
//     return true;
//   }
//   return false;
}

void DupDetector::FindDD_(const std::vector<FpItem>& data, const FpTable& table, uint32_t table_id)
{
    std::vector<FpItem > for_compare;
    bool is_first = true;
    bool has_new = false;
    uint64_t last_compare_value = 0;
    uint32_t total_count = data.size();
    uint32_t dd_count = 0;
    for(uint32_t i=0;i<total_count;i++)
    {
        //     if(i%10000==0) std::cout<<"[dup-id] "<<i<<std::endl;
        uint64_t compare_value = table.GetBitsValue(data[i].fp);
        if( is_first )
        {
        }
        else
        {
            if( last_compare_value != compare_value )
            {
                //process for_compare
                if( has_new ) 
                {
                    if(!SkipTrash_(total_count, for_compare.size(), table_id))
                    PairWiseCompare_(for_compare, dd_count);
                }
                for_compare.resize(0);
                has_new = false;
            }
        }
        for_compare.push_back(data[i]);
        last_compare_value = compare_value;

        if( data[i].status>0 )
        {
            has_new = true;
        }
        is_first = false;
    }
    if( has_new ) 
    {
        if(!SkipTrash_(total_count, for_compare.size(), table_id))
            PairWiseCompare_(for_compare, dd_count);
    }
    std::cout<<"table id "<<table_id<<" find "<<dd_count<<" dd pairs."<<std::endl;
}

void DupDetector::PairWiseCompare_(const std::vector<FpItem>& for_compare, uint32_t& dd_count)
{
    if(for_compare.size()==0) return;
    #ifdef DUPD_GROUP_DEBUG
    std::cout<<"[in-group] "<<for_compare.size()<<std::endl;
    #endif
    for( uint32_t i=0;i<for_compare.size();i++)
    {
        for(uint32_t j=i+1;j<for_compare.size();j++)
        {
            if( for_compare[i].status==0 && for_compare[j].status==0 ) continue;
            if( IsDuplicated_(for_compare[i], for_compare[j] ) )
            {
                ++dd_count;
                boost::lock_guard<boost::shared_mutex> lock(read_write_mutex_);
                group_table_->AddDoc( for_compare[i].docid, for_compare[j].docid );

            }

        }
    }
}

bool DupDetector::IsDuplicated_(const FpItem& item1, const FpItem& item2)
{
  izenelib::util::CBitArray c = item1.fp ^ item2.fp;
  bool result = false;

  // K should be selected by longer document. (longer document generates lower K value)
  // Therefore, std::max is used to get longer document.
  uint32_t doc_length = std::max(item1.length, item2.length);

  // We need to check the corpus is based on English alphabet.
  // Because GetK_ function is based on actual document length.
  // In here, 7 is multiplied to document length. (7 means the average word length in Enlgish)
//   bool isEnglish = true;
//   if(isEnglish)
    
  doc_length *= 7;
  uint8_t k = GetK_(doc_length);
  if( c.GetCount() <= k )
  {
    result = true;
#ifdef NO_IN_SITE
    std::string url_property = "Url";
    std::string url1;
    std::string url2;
    GetPropertyString_(item1.docid, url_property, url1);
    GetPropertyString_(item2.docid, url_property, url2);
    
    std::string base_site1;
    std::string base_site2;
    if(!izenelib::util::UrlUtil::GetBaseSite(url1, base_site1))
    {
        std::cout<<"get base site error for "<<url1<<std::endl;
        return false;
    }
    if(!izenelib::util::UrlUtil::GetBaseSite(url2, base_site2))
    {
        std::cout<<"get base site error for "<<url2<<std::endl;
        return false;
    }
//     std::cout<<"SITE: "<<url1<<" -> "<<base_site1<<std::endl;
//     std::cout<<"SITE: "<<url2<<" -> "<<base_site2<<std::endl;
    if(base_site1==base_site2) result = false;
#endif
  }
#ifdef DUPD_DEBUG
  if(result)
  {
    std::cout<<"find dd: "<<item1.docid<<" , "<<item2.docid<<" : "<<(uint32_t)k<<","<<c.GetCount()<<std::endl;
  }
#endif
//   std::cout<<"D"<<item1.docid<<",D"<<item2.docid<<" "<<c.GetCount()<<" "<<item1.length<<","<<item2.length<<" "<<(int)result<<std::endl;
  return result;
}


// void DupDetector2::OutputResult_(const std::string& file)
// {
//   std::string title_property = "Title";
//   std::string url_property = "Url";
//   std::ofstream ofs( file.c_str() );
//   const std::vector<std::vector<uint32_t> >& group_info = group_->GetGroupInfo();
//   std::cout<<"[TOTAL GROUP SIZE] : "<<group_info.size()<<std::endl;
//   for(uint32_t group_id = 0;group_id<group_info.size();group_id++)
//   {
//       const std::vector<uint32_t>& in_group = group_info[group_id];
//       for(uint32_t i=0;i<in_group.size();i++)
//       {
//           uint32_t docid = in_group[i];
//           std::string title;
//           std::string url;
//           GetPropertyString_(docid, title_property, title);
//           GetPropertyString_(docid, url_property, url);
//           ofs<<docid<<"\t"<<title<<"\t"<<url<<std::endl;
//       }
//       ofs<<std::endl<<std::endl;
//   }
//   ofs.close();
// }

DupDetector::FpWriterType* DupDetector::GetFpWriter_()
{
    if(writer_==NULL)
    {
        writer_ = new FpWriterType(fp_working_path_);
        if(!writer_->Open())
        {
            delete writer_;
            writer_ = NULL;
        }
    }
    return writer_;
}

void DupDetector::InsertDoc(const DocIdType& docid, const std::vector<std::string>& v)
{
    FpWriterType* writer = GetFpWriter_();
    if(writer==NULL)
    {
        std::cout<<"writer null"<<std::endl;
        return;
    }
    izenelib::util::CBitArray bit_array;
    //  CharikarAlgorithm algo;
    algo_->generate_document_signature(v, bit_array);
    FpItem fpitem(docid, bit_array, v.size());
    writer->Append(fpitem);
}

void DupDetector::RemoveDoc(const DocIdType& docid)
{
    //TODO
    group_table_->RemoveDoc(docid);
}
  
bool DupDetector::RunDdAnalysis()
{
    std::vector<FpItem> vec;
    if(writer_!=NULL)
    {
        writer_->Close();
        delete writer_;
        writer_ = NULL;
    }
    izenelib::am::ssf::Util<>::Load(fp_working_path_, vec);
    if(vec.empty())
    {
        std::cout<<"no new data"<<std::endl;
        return true;
    }
    for(uint32_t i=0;i<vec.size();i++)
    {
        vec[i].status = 1; //set status= new
    }
    uint32_t table_count = table_list_.size();
    
    std::vector<FpItem> vec_all;
    izenelib::am::ssf::Util<>::Load(fp_storage_path_, vec_all);
    std::cout<<"Processed doc count : "<<vec_all.size()<<std::endl;
    vec_all.insert(vec_all.end(), vec.begin(), vec.end());
    vec.clear();
    if(!izenelib::am::ssf::Util<>::Save(fp_storage_path_, vec_all))
    {
        std::cerr<<"Save fps failed"<<std::endl;
    }
    boost::filesystem::remove_all(fp_working_path_);//delete working data
    #ifdef DUPD_FP_DEBUG
    for(uint32_t i=0;i<vec_all.size();i++)
    {
        const FpItem& item = vec_all[i];
        std::cout<<"FP "<<item.docid<<","<<item.length<<",";
        item.fp.display(std::cout);
        std::cout<<std::endl;
    }
    #endif
    std::cout<<"Now all doc count : "<<vec_all.size()<<std::endl;
    std::cout<<"Table count : "<<table_count<<std::endl;

    for(uint32_t tid=0;tid<table_count;tid++)
    {
        MEMLOG("Processing table id : %d", tid);
        FpTable table = table_list_[tid];
        std::sort( vec_all.begin(), vec_all.end(), table );
        FindDD_(vec_all, table, tid);
    }
    if(!group_table_->Flush())
    {
        std::cerr<<"group flush error"<<std::endl;
        return false;
    }
    group_table_->PrintStat();
    //output to text file for manually review
    //   std::string output_txt_file = container_+"/output_group_dd.txt";
    //   OutputResult_(output_txt_file);
    return true;
}



