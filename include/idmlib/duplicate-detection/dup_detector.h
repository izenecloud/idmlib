#ifndef IDMLIB_DD_DUPDETECTOR_H_
#define IDMLIB_DD_DUPDETECTOR_H_

#include <idmlib/idm_types.h>
#include "dd_types.h"
#include "charikar_algo.h"
#include "group_table.h"
#include "fp_tables.h"
#include <string>
#include <vector>

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

#include <am/sequence_file/ssfr.h>
#include <idmlib/util/file_object.h>
#include <idmlib/util/idm_analyzer.h>
NS_IDMLIB_DD_BEGIN

class DupDetector
{

public:
    typedef izenelib::am::ssf::Writer<> FpWriterType;
    
    DupDetector(const std::string& container, GroupTable* group_table);
    
    ~DupDetector();

    bool Open();

    void InsertDoc(const DocIdType& docid, const std::vector<std::string>& v);
    
    void RemoveDoc(const DocIdType& docid);

    bool RunDdAnalysis();

private:
    
    FpWriterType* GetFpWriter_();
        
    
    void FindDD_(const std::vector<FpItem>& object, const FpTable& table, uint32_t table_id);

    bool SkipTrash_(uint32_t total_count, uint32_t pairwise_count, uint32_t table_id);

    void PairWiseCompare_(const std::vector<FpItem>& for_compare, uint32_t& dd_count);

    bool IsDuplicated_(const FpItem& item1, const FpItem& item2);

    uint8_t GetK_(uint32_t doc_length);

    void SetParameters_();
    
    void FindSim_(uint32_t docid1, uint32_t docid2, double score);
    
    void OutputResult_(const std::string& file);


//     void DataDupd(const std::vector<std::pair<uint32_t, izenelib::util::CBitArray> >& data, uint8_t group_num, uint32_t min_docid);


private:

    std::string container_;
    
    /// parameters.
    uint8_t maxk_;
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
    
    GroupTable* group_table_;

    boost::shared_mutex read_write_mutex_;

};

NS_IDMLIB_DD_END

#endif /* DUPDETECTOR_H_ */
