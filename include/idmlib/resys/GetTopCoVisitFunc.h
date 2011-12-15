#ifndef IDMLIB_RESYS_GET_TOP_COVISIT_FUNC_H
#define IDMLIB_RESYS_GET_TOP_COVISIT_FUNC_H

#include "CoVisitFreq.h"
#include "ItemRescorer.h"
#include <util/PriorityQueue.h>

#include <vector>

NS_IDMLIB_RESYS_BEGIN

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
        const CoVisitationQueueItem<CoVisitation>& o1,
        const CoVisitationQueueItem<CoVisitation>& o2
    ) const
    {
        return (o1.covisitation.freq < o2.covisitation.freq);
    }
};

template <typename CoVisitation, typename RowType>
class GetTopCoVisitFunc
{
public:
    GetTopCoVisitFunc(
        ItemType rowItem,
        ItemRescorer* filter,
        std::size_t topCount
    )
    : rowItem_(rowItem)
    , filter_(filter)
    , queue_(topCount)
    {}

    void operator() (const RowType& row)
    {
        for(typename RowType::const_iterator iter = row.begin();
            iter != row.end(); ++iter)
        {
            const ItemType item = iter->first;
            const CoVisitation& value = iter->second;

            if (item != rowItem_ &&
                (!filter_ || !filter_->isFiltered(item)))
            {
                queue_.insert(CoVisitationQueueItem<CoVisitation>(item, value));
            }
        }
    }

    void getResult(std::vector<ItemType>& results)
    {
        results.resize(queue_.size());
        for (std::vector<ItemType>::reverse_iterator rit = results.rbegin();
            rit != results.rend(); ++rit)
        {
            *rit = queue_.pop().itemId;
        }
    }

private:
    const ItemType rowItem_;
    ItemRescorer* filter_;
    CoVisitationQueue<CoVisitation> queue_;
};

NS_IDMLIB_RESYS_END

#endif
