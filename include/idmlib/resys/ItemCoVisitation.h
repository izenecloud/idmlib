#ifndef IDMLIB_RESYS_ITEM_COVISITATION_H
#define IDMLIB_RESYS_ITEM_COVISITATION_H

#include "ItemRescorer.h"
#include "SerializationType.h"
#include <idmlib/idm_types.h>

#include <am/beansdb/Hash.h>
#include <am/leveldb/Table.h>
#include <sdb/SequentialDB.h>
#include <cache/IzeneCache.h>

#include <util/Int2String.h>
#include <util/ThreadModel.h>
#include <util/PriorityQueue.h>

#include <boost/shared_ptr.hpp>

#include <string>
#include <vector>
#include <list>
#include <map>
#include <algorithm>

#include <sys/time.h>

using namespace std;

NS_IDMLIB_RESYS_BEGIN

using izenelib::util::Int2String;

typedef uint32_t ItemType;


template<typename CoVisitation>
struct CoVisitationQueueItem
{
    CoVisitationQueueItem()  {}
    CoVisitationQueueItem(ItemType item, CoVisitation covisit)
        :itemId(item),covisitation(covisit){}
    ItemType itemId;
    CoVisitation covisitation;
};

template<typename CoVisitation>
class CoVisitationQueue : public izenelib::util::PriorityQueue<CoVisitationQueueItem<CoVisitation> >
{
public:
    CoVisitationQueue(size_t size)
    {
        this->initialize(size);
    }
protected:
    bool lessThan(CoVisitationQueueItem<CoVisitation> o1, CoVisitationQueueItem<CoVisitation> o2)
    {
        return (o1.covisitation.freq < o2.covisitation.freq);
    }
};

template<typename CoVisitation>
class ItemCoVisitation
{
    //typedef __gnu_cxx::hash_map<ItemType, CoVisitation> HashType;
    typedef google::sparse_hash_map<ItemType, CoVisitation> HashType;

    //typedef izenelib::am::beansdb::Hash<Int2String, HashType > StorageType;
    //typedef izenelib::am::leveldb::Table<Int2String, HashType > StorageType;
    typedef izenelib::sdb::unordered_sdb_tc<ItemType, HashType, ReadWriteLock > StorageType;

    typedef izenelib::cache::IzeneCache<
    ItemType,
    boost::shared_ptr<HashType >,
    izenelib::util::ReadWriteLock,
    izenelib::cache::RDE_HASH,
    izenelib::cache::LFU
    > RowCacheType;

public:
    ItemCoVisitation(
          const std::string& homePath, 
          size_t row_cache_size = 100
          )
        : store_(homePath)
        , row_cache_(row_cache_size)
    {
        store_.open();
    }

    ~ItemCoVisitation()
    {
        store_.flush();
        store_.close();
    }

    /**
     * Update items visited by one user.
     * @param oldItems the items visited before
     * @param newItems the new visited items
     *
     * As the covisit matrix records the number of users who have visited both item i and item j,
     * the @c visit function has below pre-conditions:
     * @pre each items in @p oldItems should be unique
     * @pre each items in @p newItems should be unique
     * @pre there should be no items contained in both @p oldItems and @p newItems
     *
     * for performance reason, it has below post-condition:
     * @post when @c visit function returns, the items in @p newItems would be moved to the end of @p oldItems,
     *       and @p newItems would be empty.
     */
    void visit(std::list<ItemType>& oldItems, std::list<ItemType>& newItems)
    {
        izenelib::util::ScopedWriteLock<izenelib::util::ReadWriteLock> lock(lock_);

        std::list<ItemType>::const_iterator iter;
        if (oldItems.empty())
        {
            // update new*new pairs
            for(iter = newItems.begin(); iter != newItems.end(); ++iter)
                updateCoVisation(*iter, newItems);

            // for post condition
            oldItems.swap(newItems);
        }
        else
        {
            // update old*new pairs
            for(iter = oldItems.begin(); iter != oldItems.end(); ++iter)
                updateCoVisation(*iter, newItems);

            // move new items into oldItems
            std::list<ItemType>::const_iterator newItemIt = oldItems.end();
            --newItemIt;
            oldItems.splice(oldItems.end(), newItems);
            ++newItemIt;

            // update new*total pairs
            for(iter = newItemIt; iter != oldItems.end(); ++iter)
                updateCoVisation(*iter, oldItems);
        }
    }

    void getCoVisitation(size_t howmany, ItemType item, std::vector<ItemType>& results, ItemRescorer* rescorer = NULL)
    {
        izenelib::util::ScopedReadLock<izenelib::util::ReadWriteLock> lock(lock_);

        HashType rowdata;
        //Int2String rowKey(item);
        store_.get(item, rowdata);
        CoVisitationQueue<CoVisitation> queue(howmany);
        typename HashType::iterator iter = rowdata.begin();
        for(;iter != rowdata.end(); ++iter)
        {
            // escape the input item
            if (iter->first != item
                && (!rescorer || !rescorer->isFiltered(iter->first)))
            {
                queue.insert(CoVisitationQueueItem<CoVisitation>(iter->first,iter->second));
            }
        }
        results.resize(queue.size());
        for(std::vector<ItemType>::reverse_iterator rit = results.rbegin(); rit != results.rend(); ++rit)
            *rit = queue.pop().itemId;
    }

    void getCoVisitation(size_t howmany, ItemType item, std::vector<ItemType>& results, int64_t timestamp, ItemRescorer* rescorer = NULL)
    {
        izenelib::util::ScopedReadLock<izenelib::util::ReadWriteLock> lock(lock_);

        HashType rowdata;
        //Int2String rowKey(item);
        store_.get(item, rowdata);
        CoVisitationQueue<CoVisitation> queue(howmany);
        typename HashType::iterator iter = rowdata.begin();
        for(;iter != rowdata.end(); ++iter)
        {
            // escape the input item
            if (iter->timestamp >= timestamp && iter->first != item
                && (!rescorer || !rescorer->isFiltered(iter->first)))
            {
                queue.insert(CoVisitationQueueItem<CoVisitation>(iter->first,iter->second));
            }
        }
        results.resize(queue.size());
        for(std::vector<ItemType>::reverse_iterator rit = results.rbegin(); rit != results.rend(); ++rit)
            *rit = queue.pop().itemId;
    }

    uint32_t coeff(ItemType row, ItemType col)
    {
        boost::shared_ptr<HashType > rowdata;
        //we do not use read lock here because coeff is called only after writing operation is finished
        //izenelib::util::ScopedReadLock<izenelib::util::ReadWriteLock> lock(lock_);
        if (!row_cache_.getValueNoInsert(row, rowdata))
        {
            rowdata = loadRow(row);
            row_cache_.insertValue(row, rowdata);
        }
        return (*rowdata)[col].freq;
    }

    void gc()
    {
        //store_.optimize();
    }

private:
    void updateCoVisation(ItemType item, const std::list<ItemType>& coItems)
    {
        if(coItems.empty()) return;
        boost::shared_ptr<HashType > rowdata;
        if (!row_cache_.getValueNoInsert(item, rowdata))
        {
            rowdata = loadRow(item);
            row_cache_.insertValue(item, rowdata);
        }

        for(std::list<ItemType>::const_iterator iter = coItems.begin();
            iter != coItems.end(); ++iter)
        {
            CoVisitation& covisitation = (*rowdata)[*iter];
            covisitation.update();
        }

        saveRow(item,rowdata);
    }

    boost::shared_ptr<HashType > loadRow(ItemType row)
    {
        boost::shared_ptr<HashType > rowdata(new HashType);
        //Int2String rowKey(row);
        store_.get(row, *rowdata);
        return rowdata;
    }

    void saveRow(ItemType row, boost::shared_ptr<HashType > rowdata)
    {
        //Int2String rowKey(row);
        store_.update(row, *rowdata);
    }

private:
    StorageType store_;
    RowCacheType row_cache_;	
    izenelib::util::ReadWriteLock lock_;
};


NS_IDMLIB_RESYS_END

#endif

