#ifndef IDMLIB_RESYS_ITEM_COVISITATION_H
#define IDMLIB_RESYS_ITEM_COVISITATION_H

#include "ItemRescorer.h"
#include <idmlib/idm_types.h>

#include <am/matrix/matrix_db.h>

#include <util/PriorityQueue.h>
#include <util/timestamp.h>

#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <algorithm>
#include <iostream>

#include <sys/time.h>

using namespace std;

NS_IDMLIB_RESYS_BEGIN

typedef uint32_t ItemType;

struct CoVisitTimeFreq :public ::izenelib::util::pod_tag
{
    CoVisitTimeFreq():freq(0),timestamp(0){}

    uint32_t freq;
    int64_t timestamp;

    void update()
    {
        freq += 1;
        timestamp = (int64_t)izenelib::util::Timestamp::now();
    }
};

struct CoVisitFreq :public ::izenelib::util::pod_tag
{
    CoVisitFreq():freq(0){}

    uint32_t freq;

    void update()
    {
        freq += 1;
    }
};

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
class CoVisitationQueue 
    : public izenelib::util::PriorityQueue<CoVisitationQueueItem<CoVisitation> >
{
public:
    CoVisitationQueue(size_t size)
    {
        this->initialize(size);
    }
protected:
    bool lessThan(
        CoVisitationQueueItem<CoVisitation> o1, 
        CoVisitationQueueItem<CoVisitation> o2
    )
    {
        return (o1.covisitation.freq < o2.covisitation.freq);
    }
};

template<typename CoVisitation>
class ItemCoVisitation
{
    typedef izenelib::am::MatrixDB<ItemType, CoVisitation > MatrixDBType;

public:
    typedef typename MatrixDBType::row_type RowType;
    typedef typename MatrixDBType::iterator iterator; // first is ItemType, second is RowType

    ItemCoVisitation(
        const std::string& homePath, 
        size_t cache_size = 1024*1024
    )
        : db_(cache_size, homePath)
    {
    }

    ~ItemCoVisitation()
    {
        flush();
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
     */
    void visit(
        const std::list<ItemType>& oldItems, 
        const std::list<ItemType>& newItems
    )
    {
        std::list<ItemType>::const_iterator iter;

        // update old*new pairs
        std::list<ItemType> emptyList;
        for(iter = oldItems.begin(); iter != oldItems.end(); ++iter)
            updateCoVisation_(*iter, newItems, emptyList);

        // update new*total pairs
        for(iter = newItems.begin(); iter != newItems.end(); ++iter)
            updateCoVisation_(*iter, oldItems, newItems);
    }

    void getCoVisitation(
        size_t howmany, 
        ItemType item, 
        std::vector<ItemType>& results, 
        ItemRescorer* rescorer = NULL
    )
    {
        boost::shared_ptr<const RowType> rowdata = db_.row(item);
        CoVisitationQueue<CoVisitation> queue(howmany);
        typename RowType::const_iterator iter = rowdata->begin();
        for(;iter != rowdata->end(); ++iter)
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
        {
            *rit = queue.pop().itemId;
        }
    }

    void getCoVisitation(
        size_t howmany, 
        ItemType item, 
        std::vector<ItemType>& results, 
        int64_t timestamp, 
        ItemRescorer* rescorer = NULL
    )
    {
        boost::shared_ptr<const RowType> rowdata = db_.row(item);

        CoVisitationQueue<CoVisitation> queue(howmany);
        typename RowType::const_iterator iter = rowdata->begin();
        for(;iter != rowdata->end(); ++iter)
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
        return db_.elem(row,col).freq;
    }

    /**
     * return the items in @p row.
     * @param row the row number
     * @return the row items
     */
    boost::shared_ptr<const RowType> rowItems(ItemType row)
    {
        return db_.row(row);
    }

    void flush()
    {
        db_.flush();
    }

    void print(std::ostream& ostream) const
    {
        db_.print(ostream);
    }

    iterator begin()
    {
        return db_.begin();
    }

    iterator end()
    {
        return db_.end();
    }

private:
    /**
     * For the row @p item, update its columns @p cols1 and @p cols2.
     * @param item the row number
     * @param cols1 column list 1
     * @param cols2 column list 2
     */
    void updateCoVisation_(
        ItemType item,
        const std::list<ItemType>& cols1,
        const std::list<ItemType>& cols2
    )
    {
        boost::shared_ptr<const RowType> oldRow = db_.row(item);
        RowType newRow(*oldRow);

        updateColumns_(newRow, cols1);
        updateColumns_(newRow, cols2);

        db_.update_row(item, newRow);
    }

    /**
     * For the @p row , update its columns @p cols.
     * @param row the row instance
     * @param cols column list to update
     */
    void updateColumns_(
        RowType& row,
        const std::list<ItemType>& cols
    ) const
    {
        typename std::list<ItemType>::const_iterator cit;
        for(cit = cols.begin(); cit != cols.end(); ++cit)
        {
            typename RowType::iterator it = row.find(*cit);
            if(it == row.end())
            {
                row[*cit].update();
            }
            else
            {
                it->second.update();
            }
        }
    }

private:
    MatrixDBType db_;
};

template<typename CoVisitation>
std::ostream& operator<<(
    std::ostream& out,
    const ItemCoVisitation<CoVisitation>& covisit
)
{
    covisit.print(out);
    return out;
}

NS_IDMLIB_RESYS_END

#endif

