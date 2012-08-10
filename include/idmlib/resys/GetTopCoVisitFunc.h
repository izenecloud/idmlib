#ifndef IDMLIB_RESYS_GET_TOP_COVISIT_FUNC_H
#define IDMLIB_RESYS_GET_TOP_COVISIT_FUNC_H

#include "CoVisitFreq.h"
#include "ItemRescorer.h"
#include "RecommendItem.h"
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
        const ItemRescorer* filter,
        uint32_t& totalFreq,
        CoVisitationQueue<CoVisitation>& queue
    )
    : rowItem_(rowItem)
    , filter_(filter)
    , totalFreq_(totalFreq)
    , queue_(queue)
    {}

    void operator() (const RowType& row)
    {
        for (typename RowType::const_iterator iter = row.begin();
            iter != row.end(); ++iter)
        {
            const ItemType item = iter->first;
            const CoVisitation& value = iter->second;

            if (item != rowItem_ &&
                (!filter_ || !filter_->isFiltered(item)))
            {
                totalFreq_ += value.freq;
                queue_.insert(CoVisitationQueueItem<CoVisitation>(item, value));
            }
        }
    }

    void getResult(RecommendItemVec& recItems)
    {
        recItems.resize(queue_.size());

        for (RecommendItemVec::reverse_iterator rit = recItems.rbegin();
            rit != recItems.rend(); ++rit)
        {
            const CoVisitationQueueItem<CoVisitation>& queueItem = queue_.pop();
            rit->itemId_ = queueItem.itemId;
            rit->weight_ = static_cast<float>(queueItem.covisitation.freq) / totalFreq_;
        }
    }

private:
    const ItemType rowItem_;
    const ItemRescorer* filter_;
    uint32_t& totalFreq_;
    CoVisitationQueue<CoVisitation>& queue_;
};

NS_IDMLIB_RESYS_END

#endif
