#include <idmlib/similarity/all-pairs-similarity-search/all_pairs_output.h>

using namespace idmlib::sim;


void DocSimOutput::getSimilarDocIdScoreList(
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
}

/// private ////

bool DocSimOutput::constructSimIndex()
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
                updateSimIndex(lastIndexId, listData, 100, db); // todo, Limit
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


bool DocSimOutput::updateSimIndex(
    uint32_t indexId,
    std::vector<value_type>& list,
    std::size_t Limit,
    const boost::shared_ptr<Hash>& db)
{
    typedef izenelib::util::second_greater<value_type> greater_than;
    std::sort(list.begin(), list.end(), greater_than());
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

