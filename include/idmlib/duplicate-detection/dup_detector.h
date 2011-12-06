#ifndef IDMLIB_DD_DUPDETECTOR_H_
#define IDMLIB_DD_DUPDETECTOR_H_

#include <idmlib/idm_types.h>
#include "dd_types.h"
#include "charikar_algo.h"
#include "group_table.h"
#include "fp_tables.h"
#include "dd_constants.h"
#include <string>
#include <vector>

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

#include <am/sequence_file/ssfr.h>
#include <util/CBitArray.h>
#include <util/ClockTimer.h>
#include <idmlib/util/file_object.h>
#include <idmlib/util/idm_analyzer.h>
NS_IDMLIB_DD_BEGIN
// #define DUPD_GROUP_DEBUG;
// #define DUPD_TEXT_DEBUG;
// #define DUPD_DEBUG;
// #define DUPD_FP_DEBUG;
// #define NO_IN_SITE;
template< typename DT, typename GT, class AttachType = NullType>
class DupDetector
{

public:
    typedef DT DocIdType;
    typedef GT GroupIdType;
    typedef izenelib::am::ssf::Writer<> FpWriterType;
    typedef GroupTable<DocIdType, GroupIdType> GroupTableType;
    typedef FpItem<DocIdType, AttachType> FpItemType;
    
    DupDetector(const std::string& container, GroupTableType* group_table)
    :container_(container)
    , maxk_(0), partition_num_(0)
    , algo_(NULL)
    , writer_(NULL)
    , group_table_(group_table)
    {
        SetParameters_();
    }
    
    ~DupDetector()
    {
        if(algo_)
        delete algo_;
    }

    bool Open()
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

        algo_ = new CharikarAlgo(DdConstants::f);
        FpTables tables;
        tables.GenTables(DdConstants::f, maxk_, partition_num_, table_list_);
        std::cout<<"Generated "<<table_list_.size()<<" tables for maxk="<<(int)maxk_<<std::endl;

        return true;
    }

    void InsertDoc(const DocIdType& docid, const std::vector<std::string>& v, const AttachType& attach)
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
        FpItemType fpitem(docid, bit_array, v.size(), attach);
        writer->Append(fpitem);
    }
    
//     void InsertDoc(const DocIdType& docid, const std::vector<std::vector<std::string> >& v_list)
//     {
//         FpWriterType* writer = GetFpWriter_();
//         if(writer==NULL)
//         {
//             std::cout<<"writer null"<<std::endl;
//             return;
//         }
//         izenelib::util::CBitArray bit_array;
//         uint32_t length = 0;
//         //  CharikarAlgorithm algo;
//         for(uint32_t i=0;i<v_list.size();i++)
//         {
//             izenelib::util::CBitArray p_bit_array;
//             algo_->generate_document_signature(v_list[i], p_bit_array);
//             if (bit_array.IsEmpty())
//             {
//                 bit_array = p_bit_array;
//             }
//             else
//             {
//                 bit_array^=p_bit_array;
//             }
//             length += v_list[i].size();
//         }
//         FpItemType fpitem(docid, bit_array, length);
//         writer->Append(fpitem);
//     }
    
    void RemoveDoc(const DocIdType& docid)
    {
        group_table_->RemoveDoc(docid);
    }

    bool RunDdAnalysis()
    {
        std::vector<FpItemType> vec;
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
        
        
        std::vector<FpItemType> vec_all;
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
            const FpItemType& item = vec_all[i];
            std::cout<<"FP "<<item.docid<<","<<item.length<<",";
            item.fp.display(std::cout);
            std::cout<<std::endl;
        }
        #endif
        
        uint32_t table_count = table_list_.size();
        if(maxt_>0 && maxt_<table_count)
        {
            table_count = maxt_;
        }
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
    
    void SetFixK(uint8_t k)
    {
        fixk_ = k;
    }
    
    
    void SetMaxProcessTable(uint32_t t)
    {
        maxt_ = t;
    }

private:
    
    FpWriterType* GetFpWriter_()
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
        
    
    void FindDD_(const std::vector<FpItemType>& data, const FpTable& table, uint32_t table_id)
    {
        std::vector<FpItemType> for_compare;
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
        LOG(INFO)<<"table id "<<table_id<<" discovered "<<dd_count<<" dd pairs."<<std::endl;
    }

    bool SkipTrash_(uint32_t total_count, uint32_t pairwise_count, uint32_t table_id)
    {
        if(table_id==0) return false; //first table.
        if(pairwise_count<trash_min_) return false;
        else return true;
    }

    void PairWiseCompare_(const std::vector<FpItemType>& for_compare, uint32_t& dd_count)
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
                    boost::lock_guard<boost::shared_mutex> lock(read_write_mutex_);
                    //if this is a new discovery
                    if(group_table_->AddDoc( for_compare[i].docid, for_compare[j].docid ) )
                    {
                        ++dd_count;
                    }

                }

            }
        }
    }
    

    bool IsDuplicated_(const FpItemType& item1, const FpItemType& item2)
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
        }
        if(result)
        {
            if(!item1.attach.dd(item2.attach)) result = false;
        }
#ifdef DUPD_DEBUG
        if(result)
        {
            LOG(INFO)<<"find dd: "<<item1.docid<<" , "<<item2.docid<<" : "<<(uint32_t)k<<","<<c.GetCount()<<std::endl;
        }
#endif
        //   std::cout<<"D"<<item1.docid<<",D"<<item2.docid<<" "<<c.GetCount()<<" "<<item1.length<<","<<item2.length<<" "<<(int)result<<std::endl;
        return result;
    }

    uint8_t GetK_(uint32_t doc_length)
    {
        // (document length, k) follows a log based regression function
        // while the document length gets larger, k value decreases.
        // 1 is added to doc_length because ln(0) is not defined (1 is a kind of smothing factor)
        //   return 3;
        if(fixk_>0) return fixk_;
        double y = -1.619429845 * log((double)(doc_length + 1)) + 17.57504447;
        if(y <= 0) y = 0;
        uint8_t k = (uint8_t)y;
        if(y-(double)k>=0.5) k++;
        if(k > 6) k = 6;
        return k;
    }

    void SetParameters_()
    {
        fixk_ = 0;
        maxk_ = 6;
        maxt_ = 0;
        partition_num_ = 9;
        trash_threshold_ = 0.02;
        trash_min_ = 200;
    }
    


private:

    std::string container_;
    
    /// parameters.
    uint8_t maxk_;
    uint8_t fixk_;
    uint32_t maxt_;
    uint8_t partition_num_;
    double trash_threshold_;
    uint32_t trash_min_;

    /**
    * @brief the near duplicate detection algorithm chosen.
    */
    CharikarAlgo* algo_;

    
    ///finger prints handler
    std::vector<FpTable> table_list_;
    std::string fp_storage_path_;
    std::string fp_working_path_;
    FpWriterType* writer_;
    
    GroupTableType* group_table_;

    boost::shared_mutex read_write_mutex_;

};

NS_IDMLIB_DD_END

#endif /* DUPDETECTOR_H_ */
