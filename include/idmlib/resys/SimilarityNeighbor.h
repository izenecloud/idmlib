#ifndef IDMLIB_RESYS_SIMILARITY_NEIGHBOR_H
#define IDMLIB_RESYS_SIMILARITY_NEIGHBOR_H

#include <idmlib/idm_types.h>

#include <sdb/SequentialDB.h>
#include <sdb/SDBCursorIterator.h>

#include <util/ThreadModel.h>
#include <util/PriorityQueue.h>

#include <string>
#include <vector>
#include <set>
#include <iostream>
#include <cassert>

NS_IDMLIB_RESYS_BEGIN

template<typename MeasureType>
struct SimilarityQueueItem
{
    SimilarityQueueItem(ItemType item = 0, MeasureType similarity = 0)
        :itemId(item),similarity(similarity)
    {}

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
    bool lessThan(const SimilarityQueueItem<MeasureType>& o1, const SimilarityQueueItem<MeasureType>& o2) const
    {
        return (o1.similarity < o2.similarity);
    }
};

template<typename ItemType = uint32_t, typename MeasureType = float>
class SimilarityNeighbor
{
private:
    typedef std::pair<ItemType, MeasureType> ItemMeasurePair;
    typedef std::vector<ItemMeasurePair> ItemNeighborType;
    typedef std::vector<ItemNeighborType> NeighborsType;
    NeighborsType neighbors_;

    typedef izenelib::sdb::unordered_sdb_tc<
        ItemType,
        ItemNeighborType,
        ReadWriteLock
        > NeighborSDBType;
    typedef izenelib::sdb::SDBCursorIterator<NeighborSDBType> NeighborSDBIterator;
    NeighborSDBType db_storage_;

    typedef izenelib::util::ReadWriteLock LockType;
    typedef izenelib::util::ScopedReadLock<LockType> ScopedReadLock;
    typedef izenelib::util::ScopedWriteLock<LockType> ScopedWriteLock;
    LockType lock_;

    const std::size_t topK_;
    std::size_t elem_count_;
    std::vector<bool> dirty_flags_;

public:
    SimilarityNeighbor(const std::string& item_neighbor_path, size_t topK = 30)
        : db_storage_(item_neighbor_path)
        , topK_(topK)
        , elem_count_(0)
    {
        db_storage_.open();
        loadAllNeighbors_();
    }

    ~SimilarityNeighbor()
    {
        flush();

        db_storage_.flush();
        db_storage_.close();
    }

    /**
     * given the items @p historySet as user's purchase history ,
     * calculate the similarity weight for the item @p candidate.
     * @param candidate calculate weight for this item
     * @param historySet the user's purchased items
     * @param reasonItems the items contained in both @p historySet and @p candidate's neighbors
     * @return the similarity weight
     */
    float weight(
        ItemType candidate,
        const std::set<ItemType>& historySet,
        std::vector<ItemType>& reasonItems
    )
    {
        if (candidate >= neighbors_.size())
            return 0;

        float sim = 0;
        float total = 0;

        ScopedReadLock lock(lock_);

        const ItemNeighborType& neighbor = neighbors_[candidate];
        typename std::set<ItemType>::const_iterator historySetEnd = historySet.end();
        for (typename ItemNeighborType::const_iterator it = neighbor.begin();
            it != neighbor.end(); ++it)
        {
            ItemType item = it->first;
            MeasureType measure = it->second;

            if (historySet.find(item) != historySetEnd)
            {
                sim += measure;
                reasonItems.push_back(item);
            }

            total += measure;
        }

        return sim ? sim/total : 0;
    }

    /**
     * given the @p historyItemWeights as user's preference weight for some items,
     * calculate the similarity weight for the item @p candidate.
     * @param candidate calculate weight for this item
     * @param historyItemWeights the user's preference weights
     * @param reasonItems the items contained in both @p historyItemWeights and @p candidate's neighbors
     * @return the similarity weight
     */
    typedef std::map<ItemType, float> ItemWeightMap;
    float weight(
        ItemType candidate,
        const ItemWeightMap& historyItemWeights,
        std::vector<ItemType>& reasonItems
    )
    {
        if (candidate >= neighbors_.size())
            return 0;

        float sim = 0;
        float total = 0;

        ScopedReadLock lock(lock_);

        const ItemNeighborType& neighbor = neighbors_[candidate];
        typename ItemWeightMap::const_iterator historyWeightsEnd = historyItemWeights.end();
        for (typename ItemNeighborType::const_iterator it = neighbor.begin();
            it != neighbor.end(); ++it)
        {
            ItemType item = it->first;
            MeasureType measure = it->second;

            typename ItemWeightMap::const_iterator findIt = historyItemWeights.find(item);
            if (findIt != historyWeightsEnd)
            {
                float findWeight = findIt->second;
                sim += measure * findWeight;

                if (findWeight > 0)
                    reasonItems.push_back(item);
            }

            total += measure;
        }

        return sim ? sim/total : 0;
    }

    template <typename RowType>
    void updateNeighbor(ItemType itemId, const RowType& rowdata)
    {
        SimilarityQueue<MeasureType> queue(topK_);
        for (typename RowType::const_iterator iter = rowdata.begin();
            iter != rowdata.end(); ++iter)
        {
            queue.insert(SimilarityQueueItem<MeasureType>(iter->first,iter->second));
        }

        updateNeighborFromQueue_(itemId, queue);
    }

    void getNeighborSet(ItemType row, std::set<uint32_t>& items)
    {
        if (row >= neighbors_.size())
            return;

        ScopedReadLock lock(lock_);
        const ItemNeighborType& neighbor = neighbors_[row];
        for (typename ItemNeighborType::const_iterator it = neighbor.begin();
            it != neighbor.end(); ++it)
        {
            items.insert(it->first);
        }
    }

    void flush()
    {
        ScopedWriteLock lock(lock_);

        assert(dirty_flags_.size() <= neighbors_.size());
        for (unsigned int i = 0; i < dirty_flags_.size(); ++i)
        {
            if (dirty_flags_[i])
            {
                db_storage_.update(i, neighbors_[i]);
            }
        }
        dirty_flags_.clear();
    }

    void print(std::ostream& ostream) const
    {
        const std::size_t neighborSize = sizeof(ItemMeasurePair) * elem_count_ +
                                         sizeof(bool) * dirty_flags_.size() / 8;

        ostream << "SimNeighbor[" << neighborSize << "]";
    }

private:
    void loadAllNeighbors_()
    {
        NeighborSDBIterator iter = NeighborSDBIterator(db_storage_);
        NeighborSDBIterator end = NeighborSDBIterator();
        int numItems = db_storage_.numItems();
        neighbors_.clear();
        neighbors_.resize(numItems);
        for (; iter != end; ++iter)
        {
            ItemType itemId = iter->first;

            ItemNeighborType& value = const_cast<ItemNeighborType&>(iter->second);
            ItemNeighborType& neighbor = getNeighbor_(itemId);
            neighbor.swap(value);
            elem_count_ += neighbor.size();
        }
    }

    ItemNeighborType& getNeighbor_(ItemType itemId)
    {
        if (itemId >= neighbors_.size())
        {
            neighbors_.resize(itemId+1);
        }
        return neighbors_[itemId];
    }

    void updateNeighborFromQueue_(ItemType itemId, SimilarityQueue<MeasureType>& queue)
    {
        ScopedWriteLock lock(lock_);

        ItemNeighborType& neighbor = getNeighbor_(itemId);

        assert(elem_count_ >= neighbor.size());
        elem_count_ -= neighbor.size();
        neighbor.resize(queue.size());
        elem_count_ += neighbor.size();

        for (typename ItemNeighborType::reverse_iterator rit = neighbor.rbegin();
            rit != neighbor.rend(); ++rit)
        {
            SimilarityQueueItem<MeasureType> item = queue.pop();
            rit->first = item.itemId;
            rit->second = item.similarity;
        }

        setDirty_(itemId);
    }

    void setDirty_(ItemType itemId)
    {
        if (itemId >= dirty_flags_.size())
        {
            dirty_flags_.resize(itemId+1);
        }
        dirty_flags_[itemId] = true;
    }
};

template<typename ItemType, typename MeasureType>
std::ostream& operator<<(
    std::ostream& out,
    const SimilarityNeighbor<ItemType, MeasureType>& simNeighbor
)
{
    simNeighbor.print(out);
    return out;
}

NS_IDMLIB_RESYS_END

#endif
