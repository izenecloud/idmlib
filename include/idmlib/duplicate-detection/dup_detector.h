#ifndef IDMLIB_DD_DUPDETECTOR_H_
#define IDMLIB_DD_DUPDETECTOR_H_

#include "dd_constants.h"
#include "dd_types.h"
#include "charikar_algo.h"
#include "fp_tables.h"
#include "group_table.h"

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

#include <am/sequence_file/ssfr.h>


NS_IDMLIB_DD_BEGIN
//#define DUPD_GROUP_DEBUG;
//#define DUPD_DEBUG;
//#define DUPD_FP_DEBUG;
template <typename DT, typename GT, class AttachType = NullType>
class DupDetector
{
public:
    typedef DT DocIdType;
    typedef GT GroupIdType;
    typedef izenelib::am::ssf::Writer<> FpWriterType;
    typedef GroupTable<DocIdType, GroupIdType> GroupTableType;
    typedef FpItem<DocIdType, AttachType> FpItemType;

    DupDetector(const std::string& container, GroupTableType* group_table, bool enable_knn = false, bool fp_only = false)
        : container_(container)
        , maxk_(0), partition_num_(0)
        , algo_(NULL)
        , writer_(NULL)
        , group_table_(group_table)
        , enable_knn_(enable_knn)
        , fp_only_(enable_knn && fp_only)
    {
        SetParameters_();
    }

    ~DupDetector()
    {
        if (algo_)
        delete algo_;
    }

    bool Open()
    {
        fp_storage_path_ = container_ + "/fp";
        fp_working_path_ = container_ + "/fp_working";
        try
        {
            boost::filesystem::create_directories(container_);
            boost::filesystem::remove_all(fp_working_path_);
        }
        catch(std::exception& ex)
        {
            std::cerr << ex.what() << std::endl;
            return false;
        }

        algo_ = new CharikarAlgo(DdConstants::f);

        if (!fp_only_)
        {
            FpTables().GenTables(DdConstants::f, maxk_, partition_num_, table_list_);
            std::cout << "Generated " << table_list_.size() << " tables for maxk =" << (int) maxk_ << std::endl;
        }

        if (enable_knn_)
        {
            boost::lock_guard<boost::shared_mutex> lock(fp_vec_mutex_);
            izenelib::am::ssf::Util<>::Load(fp_storage_path_, fp_vec_);
        }

        return true;
    }

    void IncreaseCacheCapacity(uint32_t capacity)
    {
        boost::lock_guard<boost::shared_mutex> lock(fp_vec_mutex_);
        working_fp_vec_.reserve(working_fp_vec_.size() + capacity);
    }

    void InsertDoc(const DocIdType& docid, const std::vector<std::string>& v, std::vector<double>& weights, const AttachType& attach = AttachType())
    {
        FpWriterType* writer = NULL;
        if (!enable_knn_)
        {
            writer = GetFpWriter_();
            if (!writer)
            {
                std::cout << "writer null" << std::endl;
                return;
            }
        }
        if (weights.size() != v.size())
        {
            weights.resize(v.size(), 1.0);
        }
        // CharikarAlgorithm algo;
        FpType fp;
        algo_->generate_document_signature(v, weights, fp);
        FpItemType fpitem(docid, v.size(), fp, attach);
        if (enable_knn_)
        {
            boost::lock_guard<boost::shared_mutex> lock(fp_vec_mutex_);
            working_fp_vec_.push_back(fpitem);
        }
        else
        {
            writer->Append(fpitem);
        }
    }

    void InsertDoc(const DocIdType& docid, const std::vector<std::string>& v, const AttachType& attach = AttachType())
    {
        std::vector<double> weights(v.size(), 1.0);
        InsertDoc(docid, v, weights, attach);
    }

    void RemoveDoc(const DocIdType& docid)
    {
        removed_docs_.push_back(docid);
        if (!fp_only_) group_table_->RemoveDoc(docid);
    }

    void FinishRemoveDocs()
    {
        if (removed_docs_.empty()) return;

        if (!enable_knn_)
        {
            izenelib::am::ssf::Util<>::Load(fp_storage_path_, fp_vec_);
        }
        if (fp_vec_.empty()) return;

        std::vector<FpItemType> new_fp_vec;
        new_fp_vec.reserve(fp_vec_.size() - removed_docs_.size());
        typename std::vector<FpItemType>::iterator it(fp_vec_.begin());
        for (uint32_t i = 0; i < removed_docs_.size(); i++)
        {
            for (; it != fp_vec_.end() && it->docid < removed_docs_[i]; ++it)
                new_fp_vec.push_back(*it);
            if (it != fp_vec_.end() && it->docid == removed_docs_[i])
                ++it;
        }
        new_fp_vec.insert(new_fp_vec.end(), it, fp_vec_.end());

        izenelib::am::ssf::Util<>::Load(fp_storage_path_, new_fp_vec);
        if (enable_knn_)
        {
            new_fp_vec.swap(fp_vec_);
        }
        else
        {
            std::vector<FpItemType>().swap(fp_vec_);
        }
        std::vector<DocIdType>().swap(removed_docs_);
    }

    bool RunDdAnalysis(bool force = false)
    {
        if (enable_knn_)
        {
            return RunDdAnalysisEnableKNN_(force);
        }
        else
        {
            return RunDdAnalysisDisableKNN_(force);
        }
    }

    bool GetDuplicatedDocIdList(const DocIdType& docId, std::vector<DocIdType>& docIdList)
    {
        if (fp_only_) return false;

        boost::lock_guard<boost::shared_mutex> lock(group_table_mutex_);
        group_table_->Find(docId, docIdList);
        if (!docIdList.empty())
        {
            for (typename std::vector<DocIdType>::iterator it = docIdList.begin();
                    it != docIdList.end(); ++it)
            {
                if (*it == docId)
                {
                    docIdList.erase(it);
                    break;
                }
            }
        }
        return true;
    }

    bool GetUniqueDocIdList(const std::vector<DocIdType>& docIdList, std::vector<DocIdType>& cleanDocs)
    {
        if (fp_only_) return false;

        boost::lock_guard<boost::shared_mutex> lock(group_table_mutex_);
        cleanDocs.reserve(docIdList.size());
        for (uint32_t i = 0; i < docIdList.size(); ++i)
        {
            bool clean = true;
            for (uint32_t j = 0; j < cleanDocs.size(); ++j)
            {
                if (group_table_->IsSameGroup(docIdList[i], cleanDocs[j]))
                {
                    clean = false;
                    break;
                }
            }
            if (clean)
            {
                cleanDocs.push_back(docIdList[i]);
            }
        }
        return true;
    }

    void GetKNNListBySignature(
            const std::vector<uint64_t>& signature,
            uint32_t count,
            uint32_t max_hamming_dist,
            std::vector<std::pair<uint32_t, DocIdType> >& knn_list)
    {
        if (!enable_knn_) return;

        knn_list.clear();
        knn_list.reserve(count);

        for (uint32_t i = 0; i < fp_vec_.size(); i++)
        {
            uint32_t hamming_dist = fp_vec_[i].calcHammingDist(&signature[0]);
            if (hamming_dist > max_hamming_dist) continue;
            if (knn_list.size() < count)
            {
                knn_list.push_back(std::make_pair(hamming_dist, fp_vec_[i].docid));
                std::push_heap(knn_list.begin(), knn_list.end());
            }
            else if (hamming_dist < knn_list[0].first)
            {
                std::pop_heap(knn_list.begin(), knn_list.end());
                knn_list.back().first = hamming_dist;
                knn_list.back().second = fp_vec_[i].docid;
                std::push_heap(knn_list.begin(), knn_list.end());
            }
        }

        std::sort_heap(knn_list.begin(), knn_list.end());
    }

    inline void SetFixK(uint8_t k)
    {
        fixk_ = k;
    }

    inline void SetMaxProcessTable(uint32_t t)
    {
        maxt_ = t;
    }

    inline const CharikarAlgo* GetCharikarAlgo() const
    {
        return algo_;
    }

private:
    FpWriterType* GetFpWriter_()
    {
        if (!writer_)
        {
            writer_ = new FpWriterType(fp_working_path_);
            if (!writer_->Open())
            {
                delete writer_;
                writer_ = NULL;
            }
        }
        return writer_;
    }

    bool RunDdAnalysisEnableKNN_(bool force)
    {
        if (!force && working_fp_vec_.empty())
        {
            std::cout << "no new data" << std::endl;
            return true;
        }
        for (uint32_t i = 0; i < working_fp_vec_.size(); i++)
        {
            working_fp_vec_[i].status = 1; //set status = new
        }

        std::cout << "Processed doc count : " << fp_vec_.size() << std::endl;
        {
            boost::lock_guard<boost::shared_mutex> lock(fp_vec_mutex_);
            fp_vec_.insert(fp_vec_.end(), working_fp_vec_.begin(), working_fp_vec_.end());
            std::vector<FpItemType>().swap(working_fp_vec_);
        }
        if (!izenelib::am::ssf::Util<>::Save(fp_storage_path_, fp_vec_))
        {
            std::cerr << "Save fps failed" << std::endl;
        }
        boost::filesystem::remove_all(fp_working_path_);//delete working data

#ifdef DUPD_FP_DEBUG
        for (uint32_t i = 0; i < fp_vec_.size(); i++)
        {
            const FpItemType& item = fp_vec_[i];
            std::cout << "FP " << item.docid << "," << item.length << ",";
            std::ostringstream oss;
            oss << hex << setfill('0');
            for (int j = item.fp.size() - 1; j >= 0; j--)
                oss << setw(16) << item.fp[j];
            std::cout << oss.str() << std::endl;
        }
#endif

        if (fp_only_) return true;

        uint32_t table_count = table_list_.size();
        if (maxt_ > 0 && maxt_ < table_count)
        {
            table_count = maxt_;
        }
        std::cout << "Now all doc count : " << fp_vec_.size() << std::endl;
        std::cout << "Table count : " << table_count << std::endl;

        std::vector<FpItemType> vec_all(fp_vec_);
        for (uint32_t tid = 0; tid < table_count; tid++)
        {
            MEMLOG("Processing table id : %d", tid);
            const FpTable& table = table_list_[tid];
            std::sort(vec_all.begin(), vec_all.end(), table);
            FindDD_(vec_all, table, tid);
        }
        if (!group_table_->Flush())
        {
            std::cerr << "group flush error" << std::endl;
            return false;
        }
        group_table_->PrintStat();
        //output to text file for manually review
        //   std::string output_txt_file = container_+"/output_group_dd.txt";
        //   OutputResult_(output_txt_file);
        return true;
    }

    bool RunDdAnalysisDisableKNN_(bool force)
    {
        if (writer_)
        {
            writer_->Close();
            delete writer_;
            writer_ = NULL;
        }
        std::vector<FpItemType> vec;
        izenelib::am::ssf::Util<>::Load(fp_working_path_, vec);
        if (!force && vec.empty())
        {
            std::cout << "no new data" << std::endl;
            return true;
        }
        for (uint32_t i = 0; i < vec.size(); i++)
        {
            vec[i].status = 1; //set status = new
        }

        std::vector<FpItemType> vec_all;
        izenelib::am::ssf::Util<>::Load(fp_storage_path_, vec_all);
        std::cout << "Processed doc count : " << vec_all.size() << std::endl;
        vec_all.insert(vec_all.end(), vec.begin(), vec.end());
        if (!izenelib::am::ssf::Util<>::Save(fp_storage_path_, vec_all))
        {
            std::cerr << "Save fps failed" << std::endl;
        }
        boost::filesystem::remove_all(fp_working_path_);//delete working data

#ifdef DUPD_FP_DEBUG
        for (uint32_t i = 0; i < vec_all.size(); i++)
        {
            const FpItemType& item = vec_all[i];
            std::cout << "FP " << item.docid << "," << item.length << ",";
            std::ostringstream oss;
            oss << hex << setfill('0');
            for (int j = item.fp.size() - 1; j >= 0; j--)
                oss << setw(16) << item.fp[j];
            std::cout << oss.str() << std::endl;
        }
#endif

        uint32_t table_count = table_list_.size();
        if (maxt_ > 0 && maxt_ < table_count)
        {
            table_count = maxt_;
        }
        std::cout << "Now all doc count : " << vec_all.size() << std::endl;
        std::cout << "Table count : " << table_count << std::endl;

        for (uint32_t tid = 0; tid < table_count; tid++)
        {
            MEMLOG("Processing table id : %d", tid);
            const FpTable& table = table_list_[tid];
            std::sort(vec_all.begin(), vec_all.end(), table);
            FindDD_(vec_all, table, tid);
        }
        if (!group_table_->Flush())
        {
            std::cerr << "group flush error" << std::endl;
            return false;
        }
        group_table_->PrintStat();
        //output to text file for manually review
        //   std::string output_txt_file = container_+"/output_group_dd.txt";
        //   OutputResult_(output_txt_file);
        return true;
    }

    void FindDD_(const std::vector<FpItemType>& data, const FpTable& table, uint32_t table_id)
    {
        bool is_first = true;
        bool has_new = false;
        FpType last_compare_value, compare_value;
        uint32_t total_count = data.size();
        uint32_t dd_count = 0;
        uint32_t start = 0, finish = 0;
        for (uint32_t i = 0; i < total_count; i++)
        {
            table.GetMaskedBits(data[i].fp, compare_value);
            if (!is_first && last_compare_value != compare_value)
            {
                if (has_new)
                {
                    if (!SkipTrash_(total_count, finish - start, table_id))
                        PairWiseCompare_(data, start, finish, dd_count);
                }
                start = finish;
                has_new = false;
            }
            ++finish;
            last_compare_value = compare_value;

            if (data[i].status > 0)
            {
                has_new = true;
            }
            is_first = false;
        }
        if (has_new)
        {
            if (!SkipTrash_(total_count, finish - start, table_id))
                PairWiseCompare_(data, start, finish, dd_count);
        }
        LOG(INFO) << "table id " << table_id << " discovered " << dd_count << " dd pairs." << std::endl;
    }

    bool SkipTrash_(uint32_t total_count, uint32_t pairwise_count, uint32_t table_id)
    {
        if (table_id == 0) return false; //first table.
        if (pairwise_count < trash_min_) return false;
        return true;
    }

    void PairWiseCompare_(const std::vector<FpItemType>& data, uint32_t start, uint32_t finish, uint32_t& dd_count)
    {
        if (start >= finish) return;
#ifdef DUPD_GROUP_DEBUG
        std::cout << "[in-group] " << finish - start << std::endl;
#endif
        for (uint32_t i = start; i < finish; i++)
        {
            for (uint32_t j = i + 1; j < finish; j++)
            {
                if (data[i].status == 0 && data[j].status == 0) continue;
                if (IsDuplicated_(data[i], data[j]))
                {
                    boost::lock_guard<boost::shared_mutex> lock(group_table_mutex_);
                    //if this is a new discovery
                    if (group_table_->AddDoc(data[i].docid, data[j].docid))
                    {
                        ++dd_count;
                    }
                }
            }
        }
    }

    bool IsDuplicated_(const FpItemType& item1, const FpItemType& item2)
    {
        uint32_t hamming_dist = item1.calcHammingDist(item2.fp.desc);
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
        if (hamming_dist <= k)
        {
            result = true;
            if (!item1.attach.dd(item2.attach)) result = false;
        }
#ifdef DUPD_DEBUG
        if (result)
        {
            LOG(INFO) << "find dd: " << item1.docid << " , " << item2.docid << " : " << (uint32_t) k << "," << hamming_dist << std::endl;
        }
#endif
        return result;
    }

    uint8_t GetK_(uint32_t doc_length)
    {
        // (document length, k) follows a log based regression function
        // while the document length gets larger, k value decreases.
        // 1 is added to doc_length because ln(0) is not defined (1 is a kind of smothing factor)
        //   return 3;
        if (fixk_ > 0) return fixk_;
        double y = -1.619429845 * log(double(doc_length + 1)) + 17.57504447;
        if (y <= 0) y = 0;
        uint8_t k = (uint8_t) y;
        if (y - double(k) >= 0.5) k++;
        if (k > 6) k = 6;
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

    std::vector<FpItemType> fp_vec_;
    std::vector<FpItemType> working_fp_vec_;
    std::vector<DocIdType> removed_docs_;

    bool enable_knn_;
    bool fp_only_;

    boost::shared_mutex group_table_mutex_;
    boost::shared_mutex fp_vec_mutex_;
};

NS_IDMLIB_DD_END

#endif /* DUPDETECTOR_H_ */
