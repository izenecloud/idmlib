#ifndef IDMLIB_RESYS_SIMILARITY_MATRIX_H
#define IDMLIB_RESYS_SIMILARITY_MATRIX_H

#include <idmlib/idm_types.h>

#include <am/matrix/matrix_db.h>
#include <sdb/SequentialDB.h>
#include <sdb/SDBCursorIterator.h>

#include <util/smallfloat.h>
#include <util/timestamp.h>
#include <util/ThreadModel.h>
#include <util/PriorityQueue.h>

#include <boost/shared_ptr.hpp>

#include <string>
#include <vector>
#include <algorithm>

NS_IDMLIB_RESYS_BEGIN

template<typename MeasureType>
struct SimilarityQueueItem
{
    SimilarityQueueItem(ItemType item = 0, MeasureType similarity = 0)
        :itemId(item),similarity(similarity){}
    ItemType itemId;
    MeasureType similarity;
};

template<typename MeasureType>
class SimilarityQueue : public izenelib::util::PriorityQueue<SimilarityQueueItem<MeasureType> >
{
public:
    SimilarityQueue(size_t size)
    {
        this->initialize(size);
    }
protected:
    bool lessThan(SimilarityQueueItem<MeasureType> o1, SimilarityQueueItem<MeasureType> o2)
    {
        return (o1.similarity < o2.similarity);
    }
};

template<typename ItemType,typename MeasureType>
bool similarityCompare (const std::pair<ItemType,MeasureType>& p1, const std::pair<ItemType,MeasureType>& p2)
{
    return (p1.second > p2.second);
}

template<typename ItemType = uint32_t, typename MeasureType = float>
class SimilarityMatrix
{
    typedef izenelib::am::MatrixDB<ItemType, MeasureType > MatrixDBType;
    typedef typename MatrixDBType::row_type RowType;

    typedef std::vector<std::pair<ItemType, MeasureType> > ItemNeighborType;

    typedef izenelib::sdb::unordered_sdb_tc<
        ItemType, 
        ItemNeighborType, 
        ReadWriteLock
        > NeighborSDBType;
    typedef izenelib::sdb::SDBCursorIterator<NeighborSDBType > NeighborSDBIterator;

    typedef std::vector<ItemNeighborType > NeighborsType;

public:
    SimilarityMatrix(
          const std::string& item_item_matrix_path, 
          size_t cache_size, 
          const std::string& item_neighbor_path, 
          size_t topK = 30
          )
        : store_(cache_size, item_item_matrix_path)
        , neighbor_store_(item_neighbor_path)
        , topK_(topK)
        , max_item_(0)
    {
        neighbor_store_.open();
        loadAllNeighbors();
    }

    ~SimilarityMatrix()
    {
        neighbor_store_.flush();
        neighbor_store_.close();
    }

    bool itemSimilarity(
            ItemType itemId, 
            std::vector<std::pair<ItemType, MeasureType> >& similarities
    )
    {
        if(itemId > max_item_) return false;
        izenelib::util::ScopedReadLock<izenelib::util::ReadWriteLock> lock(neighbor_lock_);
        ItemNeighborType& neighbor = neighbors_[itemId];
        similarities.resize(neighbor.size());
        std::copy(neighbor.begin(), neighbor.end(), similarities.begin());
        return true;
    }

    float itemSimilarities(
            ItemType itemId, 
            std::map<ItemType, float>& similarities
    )
    {
        izenelib::util::ScopedReadLock<izenelib::util::ReadWriteLock> lock(neighbor_lock_);
        if(itemId >= neighbors_.size()) 
            loadNeighbor(itemId);

        ItemNeighborType& neighbor = neighbors_[itemId];
        float v = 0;
        typename ItemNeighborType::iterator it = neighbor.begin();
        for(; it != neighbor.end(); ++it)
        {
            float myv = izenelib::util::SmallFloat::byte315ToFloat(it->second);
            similarities[it->first] = myv;
            v += myv;
        }
        return v;
    }

    void adjustNeighbor(
            ItemType itemId, 
            std::list<std::pair<ItemType, MeasureType> >& newValues
    )
    {
        if(itemId >= neighbors_.size()) 
        {
            izenelib::util::ScopedWriteLock<izenelib::util::ReadWriteLock> lock(neighbor_lock_);        
            loadNeighbor(itemId);
        }
        else
        {
            ///only need to adjust in-memory neighbor data
            ItemNeighborType& neighbor = neighbors_[itemId];
            {
            izenelib::util::ScopedWriteLock<izenelib::util::ReadWriteLock> lock(neighbor_lock_);
            if(neighbor.empty())
            {
                typename std::list<std::pair<ItemType, MeasureType> >::iterator iter = newValues.begin();
                for(;iter != newValues.end(); ++iter)
                {
                    neighbor.push_back(*iter);
                }
            }
            else
            {
                typename std::list<std::pair<ItemType, MeasureType> >::iterator iter = newValues.begin();
                for(;iter != newValues.end(); ++iter)
                {
                    bool isFind = false;
                    for(typename ItemNeighborType::iterator nbIt = neighbor.begin();
                       nbIt != neighbor.end(); ++nbIt)
                    {
                        if (nbIt->first == iter->first)
                        {
                            nbIt->second = iter->second;
                            isFind = true;
                            break;
                        }
                    }

                    if(isFind == false)
                    {
                        neighbor.push_back(*iter);
                    }
                }
            }
            std::sort(neighbor.begin(), neighbor.end(),similarityCompare<ItemType,MeasureType>);
            if(neighbor.size() > topK_)
                neighbor.resize(topK_);
            }
            neighbor_store_.update(itemId,neighbor);
        }
    }

    MeasureType coeff(ItemType row, ItemType col)
    {
        return store_.coeff(row,col);
    }

    void coeff(ItemType row, ItemType col, MeasureType measure)
    {
        store_.coeff(row,col,measure);
    }

    void dump()
    {
        store_.dump();
    }

private:
    void loadAllNeighbors()
    {
        NeighborSDBIterator iter = NeighborSDBIterator(neighbor_store_);
        NeighborSDBIterator end = NeighborSDBIterator();
        int numItems = neighbor_store_.numItems();
        neighbors_.clear();
        neighbors_.resize(numItems);
        max_item_ = numItems;
        for(; iter != end; ++iter)
        {
            ItemType itemId = iter->first;
            if(itemId >= neighbors_.size())
            {
                neighbors_.resize(itemId+1);
                max_item_ = itemId;
            }

            ItemNeighborType& value = const_cast<ItemNeighborType&>(iter->second);
            ItemNeighborType& neighbor = neighbors_[itemId];
            neighbor.swap(value);
        }
    }

    void loadNeighbor(ItemType itemId)
    {
        ///need to load neighbor data from disk
        boost::shared_ptr<RowType> rowdata = store_.row(itemId);		
        SimilarityQueue<MeasureType> queue(topK_);
        typename RowType::iterator iter = rowdata->begin();
        for(;iter != rowdata->end(); ++iter)
            queue.insert(SimilarityQueueItem<MeasureType>(iter->first,iter->second));
        if(itemId >= neighbors_.size())
        {
            neighbors_.resize(itemId+1);
            max_item_ = itemId;
        }
        ItemNeighborType& neighbor = neighbors_[itemId];
        neighbor.resize(queue.size());
        for(typename ItemNeighborType::reverse_iterator rit = neighbor.rbegin(); rit != neighbor.rend(); ++rit)
        {
            SimilarityQueueItem<MeasureType> item = queue.pop();
            rit->first = item.itemId;
            rit->second = item.similarity;
        }
        neighbor_store_.update(itemId,neighbor);
    }

private:
    MatrixDBType store_;
    NeighborSDBType neighbor_store_;
    NeighborsType neighbors_;
    size_t topK_;
    ItemType max_item_;
    izenelib::util::ReadWriteLock neighbor_lock_;	
};


NS_IDMLIB_RESYS_END

#endif

