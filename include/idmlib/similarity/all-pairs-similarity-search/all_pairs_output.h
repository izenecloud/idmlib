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
        std::vector<std::pair<uint32_t, float> >& result)
    {
        result.clear();

        //boost::lock_guard<boost::shared_mutex> lg(mutex_);
        if (!db_->is_open() && !db_->open())
        {
            std::cerr << "similarity index db is not opened" << std::endl;
            return ;
        }

        Buffer key(reinterpret_cast<char*>(&documentId), sizeof(uint32_t));
        Buffer value;
        if (db_->get(key, value))
        {
            value_type* first = reinterpret_cast<value_type*>(value.data());
            std::size_t size = value.size() / sizeof(value_type);
            value_type* last = first + size;
            if (maxNum < size)
            {
                last = first + maxNum;
            }

            std::cout << "Found " << size << " similar documents for document " << documentId << std::endl;
            std::copy(first, last, std::back_inserter(result));
        }
        else
        {
            std::cout << "No similarity documents for document "
                      << documentId << std::endl;
        }

        return;
    };

private:
    bool constructSimIndex()
    {
        std::cout<<"Start to construct similarity index."<<std::endl;

        if(!sorter_->begin())
        {
            std::cout<<"No data in sorter!!!"<<std::endl;
            return false;
        }

        std::string fileName = outDir_+"/similarity.idx";
        boost::shared_ptr<Hash> db(new Hash(fileName));
        // set db cache size..
        db->open();

        std::vector<value_type> listData;
        uint32_t lastIndexId = 0;

        char* data = NULL;
        uint8_t len = 0;
        while (sorter_->next_data(len, &data))
        {
            uint32_t id1, id2;
            float weight;
            sortmeta2pair(data, id1, id2, weight);

            //std::cout<<"<"<<id1<<" , "<<id2<<">: "<<weight<<std::endl;

            if (id1 != lastIndexId)
            {
                if (lastIndexId != 0)
                    updateSimIndex(lastIndexId, listData, 100, db);
                listData.clear();
                lastIndexId = id1;
            }

            listData.push_back(value_type(id2, weight));

            free (data);
        }

        updateSimIndex(lastIndexId, listData, 100, db); // last

        db->close();

        sorter_->clear_files();
        delete sorter_; // release memory
        sorter_=new izenelib::am::IzeneSort<uint32_t, uint8_t, true>("./pair.sort", 100000000);

        std::cout<<"End to construct similarity index."<<std::endl;

        return true;
    }

    bool updateSimIndex(
        uint32_t indexId,
        std::vector<value_type>& list,
        std::size_t Limit,
        const boost::shared_ptr<Hash>& db)
    {
        typedef izenelib::util::second_greater<value_type> greater_than;
        std::sort(list.begin(), list.end(),
                  greater_than());
        if (list.size() > Limit)
        {
            list.resize(Limit);
        }
#ifdef DOC_SIM_TEST
        std::cout <<"updateSimIndex: "<<indexId<<" list: ";
        for (size_t i =0; i <list.size(); i++)
            cout <<"("<<list[i].first<<","<<list[i].second<<") ";
        cout << endl;
#endif
        if (!db->is_open() && !db->open())
        {
            return false;
        }

        izenelib::am::raw::Buffer keyBuffer(
            (char*)&indexId,
            sizeof(uint32_t)
        );
        izenelib::am::raw::Buffer valueBuffer(
            (char*)list.data(),
            list.size() * sizeof(value_type)
        );

        bool ret = db->update(keyBuffer, valueBuffer);

        return ret;
    }

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
