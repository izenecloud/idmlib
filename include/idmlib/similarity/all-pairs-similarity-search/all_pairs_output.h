/**
 * @file all_pairs_output.h
 * @author Zhongxia Li
 * @date Jun 7, 2011
 * @brief 
 */
#ifndef ALL_PAIRS_OUTPUT_H_
#define ALL_PAIRS_OUTPUT_H_

#include <string.h>

#include <idmlib/idm_types.h>

#include <boost/assert.hpp>
#include <am/tc/raw/Hash.h>
#include <am/external_sort/izene_sort.hpp>
#include <am/sdb_hash/sdb_fixedhash.h>
#include <util/functional.h>



NS_IDMLIB_SIM_BEGIN

class AllPairsOutput
{
public:
    virtual ~AllPairsOutput() {}

public:
    virtual void putPair(uint32_t id1, uint32_t id2, float weight) = 0;

    virtual void finish() = 0;
};

class DocSimOutput : public AllPairsOutput
{
    typedef izenelib::am::tc::raw::Hash Hash;
    typedef izenelib::am::raw::Buffer Buffer;
//    typedef detail::SimilarityIndexHugeDB::type HugeDB;
    typedef izenelib::am::sdb_fixedhash<uint32_t, uint8_t> INT_FIXED_HASH;

    typedef std::pair<uint32_t, float> value_type;

public:
    DocSimOutput(const std::string& outDir)
    : outDir_(outDir)
    {
        if (!boost::filesystem::exists(outDir)) {
            boost::filesystem::create_directories(outDir);
            std::cout <<"** create: "<<outDir<<endl;
        }

        string sortfile = outDir+"/pair.sort";
        boost::filesystem::remove(sortfile); //
        sorter_ = new izenelib::am::IzeneSort<uint32_t, uint8_t, true>(sortfile.c_str(), 100000000);

        metalen_ = sizeof(uint32_t)*2+sizeof(float);
        metadata_ = (char*)malloc(metalen_);
        //memset(metadata, 0, dataLen);

        std::string simidxfile = outDir_+"/similarity.idx";
        db_.reset(new Hash(simidxfile));
    }

    ~DocSimOutput()
    {
        if(sorter_)
        {
            delete sorter_;
            sorter_ = 0;
        }

        free(metadata_);
    }

public:
    void putPair(uint32_t id1, uint32_t id2, float weight)
    {
        pair2sortmeta(id1, id2, weight, metadata_);
        sorter_->add_data(metalen_, metadata_);
    }

    void finish()
    {
        sorter_->sort();

        constructSimIndex();
    }

    /// TODO
    void getSimilarDocIdScoreList(
        uint32_t documentId,
        unsigned maxNum,
        std::vector<std::pair<uint32_t, float> >& result);


private:
    bool constructSimIndex();

    bool updateSimIndex(
        uint32_t indexId,
        std::vector<value_type>& list,
        std::size_t Limit,
        const boost::shared_ptr<Hash>& db);

    /**
     * @param id1[IN]
     * @param id2[IN]
     * @param weight[IN]
     * @param metadata[OUT]
     */
    void pair2sortmeta(
            uint32_t id1,
            uint32_t id2,
            float weight,
            char* metadata)
    {
        char* pData = metadata;
        memcpy(pData, &(id1), sizeof(id1));
        pData += sizeof(id1);
        memcpy(pData, &(id2), sizeof(id2));
        pData += sizeof(id2);
        memcpy(pData, &(weight), sizeof(weight));
    }

    /**
     *
     * @param metadata[IN]
     * @param id1
     * @param id2
     * @param weight
     */
    void sortmeta2pair(
            char* metadata,
            uint32_t& id1,
            uint32_t& id2,
            float& weight)
    {
        char* pData = metadata;
        memcpy(&id1, pData, sizeof(id1));
        pData += sizeof(id1);
        memcpy(&id2, pData, sizeof(id2));
        pData += sizeof(id2);
        memcpy(&weight, pData, sizeof(weight));
    }

private:
    string outDir_;

    izenelib::am::IzeneSort<uint32_t, uint8_t, true>* sorter_;
    char* metadata_; // metadata for sorter
    uint8_t metalen_;

    boost::shared_ptr<Hash> db_;
};

NS_IDMLIB_SIM_END

#endif /* ALL_PAIRS_OUTPUT_H_ */
