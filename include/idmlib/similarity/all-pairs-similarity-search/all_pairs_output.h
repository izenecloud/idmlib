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

#include <am/external_sort/izene_sort.hpp>


NS_IDMLIB_SIM_BEGIN

class AllPairsOutput
{
public:
    AllPairsOutput()
    {
        sorter_ = new izenelib::am::IzeneSort<uint32_t, uint8_t, true>("./pair.sort", 100000000);

        metalen_ = sizeof(uint32_t)*2+sizeof(float);
        metadata_ = (char*)malloc(metalen_);
        //memset(metadata, 0, dataLen);
    }

    ~AllPairsOutput()
    {
        if(sorter_)
        {
            delete sorter_;
            sorter_ = 0;
        }

        free(metadata_);
    }

public:
    void addPair(uint32_t id1, uint32_t id2, float weight)
    {
        pair2sortmeta(id1, id2, weight, metadata_);
        sorter_->add_data(metalen_, metadata_);
    }

    void finish()
    {
        sorter_->sort();

        constructSimIndex();
    }

private:

    void constructSimIndex()
    {
        if(!sorter_->begin())
        {
            std::cout<<"No data in sorter!!!"<<std::endl;
            return;
        }

        char* data = NULL;
        uint8_t len = 0;
        while (sorter_->next_data(len, &data))
        {
           uint32_t id1, id2;
           float weight;
           sortmeta2pair(data, id1, id2, weight);

           std::cout<<"<"<<id1<<" , "<<id2<<">: "<<weight<<std::endl;

           free (data);
        }

        sorter_->clear_files();
        delete sorter_; // release memory
        sorter_=new izenelib::am::IzeneSort<uint32_t, uint8_t, true>("./pair.sort", 100000000);
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
    izenelib::am::IzeneSort<uint32_t, uint8_t, true>* sorter_;
    char* metadata_; // metadata for sorter
    uint8_t metalen_;
};

NS_IDMLIB_SIM_END

#endif /* ALL_PAIRS_OUTPUT_H_ */
