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
#include <set>
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
public:
    typedef izenelib::am::MatrixDB<ItemType, MeasureType > MatrixDBType;
    typedef typename MatrixDBType::row_type RowType;

private:
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
    {
        neighbor_store_.open();
        loadAllNeighbors();
    }

    ~SimilarityMatrix()
    {
        dump();

        neighbor_store_.flush();
        neighbor_store_.close();
    }

    bool itemSimilarity(
            ItemType itemId, 
            std::vector<std::pair<ItemType, MeasureType> >& similarities
    )
    {
        if(itemId >= neighbors_.size())
            return false;

        izenelib::util::ScopedReadLock<izenelib::util::ReadWriteLock> lock(neighbor_lock_);
        ItemNeighborType& neighbor = neighbors_[itemId];
        similarities.resize(neighbor.size());
        std::copy(neighbor.begin(), neighbor.end(), similarities.begin());
        return true;
    }

    /**
     * given the items @p historySet as user's purchase history ,
     * calculate the similarity weight for the item @p candidate.
     * @param candidate calculate weight for this item
     * @param historySet the user's purchased items
     * @return the similarity weight
     */
    float weight(
        ItemType candidate, 
        const std::set<ItemType>& historySet
    )
    {
        if(candidate >= neighbors_.size())
            return 0;

        float sim = 0;
        float total = 0;

        izenelib::util::ScopedReadLock<izenelib::util::ReadWriteLock> lock(neighbor_lock_);

        const ItemNeighborType& neighbor = neighbors_[candidate];
        for(typename ItemNeighborType::const_iterator it = neighbor.begin();
           it != neighbor.end(); ++it)
        {
            if (historySet.find(it->first) != historySet.end())
                sim += it->second;

            total += it->second;
        }

        if (sim != 0)
            return sim / total;

        return 0;
    }

    void loadNeighbor(ItemType itemId)
    {
        /// load row data from disk to queue
        boost::shared_ptr<const RowType> rowdata = store_.row(itemId);		
        SimilarityQueue<MeasureType> queue(topK_);
        typename RowType::const_iterator iter = rowdata->begin();
        for(;iter != rowdata->end(); ++iter)
            queue.insert(SimilarityQueueItem<MeasureType>(iter->first,iter->second));

        /// get topk neighbors
        {
            izenelib::util::ScopedWriteLock<izenelib::util::ReadWriteLock> lock(neighbor_lock_);

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

            /// set dirty flag
            if (itemId >= dirtyNeighbors_.size())
            {
                dirtyNeighbors_.resize(itemId+1);
            }
            dirtyNeighbors_[itemId] = true;
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

    /**
     * return the items in @p row.
     * @param row the row number
     * @return the row items
     */
    boost::shared_ptr<const RowType> rowItems(ItemType row)
    {
        return store_.row(row);
    }

    /**
     * update the items in @p row with columns @p cols.
     * @param row the row number
     * @param cols the column items
     */
    void updateRowItems(ItemType row, const RowType& cols)
    {
        return store_.update_row_without_cache(row, cols);
    }

    void dump()
    {
        store_.dump();

        {
            izenelib::util::ScopedReadLock<izenelib::util::ReadWriteLock> lock(neighbor_lock_);

            assert(dirtyNeighbors_.size() <= neighbors_.size());
            for (unsigned int i = 0; i < dirtyNeighbors_.size(); ++i)
            {
                if (dirtyNeighbors_[i])
                {
                    neighbor_store_.update(i, neighbors_[i]);
                    dirtyNeighbors_[i] = false;
                }
            }
        }
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

private:
    MatrixDBType store_;
    NeighborSDBType neighbor_store_;
    NeighborsType neighbors_;
    size_t topK_;
    ItemType max_item_;
    izenelib::util::ReadWriteLock neighbor_lock_;	
    /**
     * dirty flags to dump from @c neighbors_ to @c neighbor_store_,
     * true for dirty and need to dump.
     */
    std::vector<bool> dirtyNeighbors_; 
};


NS_IDMLIB_RESYS_END

#endif

