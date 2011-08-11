#include <idmlib/resys/incremcf/IncrementalItemCF.h>

#include <util/PriorityQueue.h>

#include <map>
#include <algorithm>
#include <cmath> // for sqrt

#include <glog/logging.h>

namespace
{

struct QueueItem
{
    QueueItem(uint32_t itemId = 0, float weight = 0)
        :itemId_(itemId)
        ,weight_(weight)
    {}

    uint32_t itemId_;
    float weight_;
};

class TopItemsQueue
    :public izenelib::util::PriorityQueue<QueueItem>
{
public:
    TopItemsQueue(size_t size)
    {
        this->initialize(size);
    }

protected:
    bool lessThan(
        QueueItem o1,
        QueueItem o2
    )
    {
        return (o1.weight_ < o2.weight_);
    }
};

}

NS_IDMLIB_RESYS_BEGIN

IncrementalItemCF::IncrementalItemCF(
    const std::string& covisit_path, 
    size_t covisit_row_cache_size,
    const std::string& item_item_similarity_path,
    size_t similarity_row_cache_size, 
    const std::string& item_neighbor_path,
    size_t topK
)
    : covisitation_(covisit_path, covisit_row_cache_size)
    , similarity_(item_item_similarity_path,similarity_row_cache_size,item_neighbor_path,topK)
{
}

IncrementalItemCF::~IncrementalItemCF()
{
}

void IncrementalItemCF::updateMatrix(
    const std::list<uint32_t>& oldItems,
    const std::list<uint32_t>& newItems
)
{
    updateVisitMatrix(oldItems, newItems);

    updateSimMatrix_(newItems);
}

void IncrementalItemCF::updateVisitMatrix(
    const std::list<uint32_t>& oldItems,
    const std::list<uint32_t>& newItems
)
{
    covisitation_.visit(oldItems, newItems);
}

void IncrementalItemCF::buildSimMatrix()
{
    LOG(INFO) << "start building similarity matrix...";

    // empty set used to call updateSimRow_()
    std::set<uint32_t> rowSet;
    std::set<uint32_t> colSet;

    int rowNum = 0;
    for (ItemCoVisitation<CoVisitFreq>::iterator it_i = covisitation_.begin();
        it_i != covisitation_.end(); ++it_i)
    {
        if (++rowNum % 1000 == 0)
        {
            std::cout << "\rbuilding row num: " << rowNum << std::flush;
        }
                                            
        const uint32_t row = it_i->first;
        const CoVisitRow& cols = it_i->second;

        updateSimRow_(row, cols, false, rowSet, colSet);
    }
    std::cout << "\rbuilding row num: " << rowNum << std::endl;

    LOG(INFO) << "finish building similarity matrix";
}

void IncrementalItemCF::updateSimRow_(
    uint32_t row,
    const CoVisitRow& coVisitRow,
    bool isUpdateCol,
    const std::set<uint32_t>& rowSet,
    std::set<uint32_t>& colSet
)
{
    uint32_t visit_i_i = 0;
    CoVisitRow::const_iterator findIt = coVisitRow.find(row);
    if (findIt != coVisitRow.end())
    {
        visit_i_i = findIt->second.freq;
    }

    SimRow simRow;
    for (CoVisitRow::const_iterator it_j = coVisitRow.begin();
        it_j != coVisitRow.end(); ++it_j)
    {
        const uint32_t col = it_j->first;

        // no similarity value on diagonal line
        if (row == col)
            continue;

        const uint32_t visit_i_j = it_j->second.freq;
        const uint32_t visit_j_j = covisitation_.coeff(col, col);
        assert(visit_i_j && visit_i_i && visit_j_j && "the freq value in visit matrix should be positive.");

        float sim = (float)visit_i_j / sqrt(visit_i_i * visit_j_j);
        simRow[col] = sim;

        // also update (col, row) if col does not exist in rowSet
        if (isUpdateCol && rowSet.find(col) == rowSet.end())
        {
            similarity_.coeff(col, row, sim);
            colSet.insert(col);
        }
    }

    similarity_.updateRowItems(row, simRow);
    similarity_.loadNeighbor(row);
}

void IncrementalItemCF::updateSimMatrix_(const std::list<uint32_t>& rows)
{
    std::set<uint32_t> rowSet(rows.begin(), rows.end());
    std::set<uint32_t> colSet;

    for (std::list<uint32_t>::const_iterator it_i = rows.begin();
        it_i != rows.end(); ++it_i)
    {
        uint32_t row = *it_i;
        boost::shared_ptr<const CoVisitRow> cols = covisitation_.rowItems(row);

        updateSimRow_(row, *cols, true, rowSet, colSet);
    }

    // update neighbor for col items
    for (std::set<uint32_t>::const_iterator it = colSet.begin();
        it != colSet.end(); ++it)
    {
        similarity_.loadNeighbor(*it);
    }
}

void IncrementalItemCF::recommend(
    int howMany,
    const std::vector<uint32_t>& visitItems,
    RecommendItemVec& recItems,
    ItemRescorer* rescorer
)
{
    typedef SimilarityMatrix<uint32_t,float>::RowType RowType;

    std::set<uint32_t> visitSet(visitItems.begin(), visitItems.end());

    // get candidate items which has similarity value with items
    std::set<uint32_t> candidateSet;
    for (std::set<uint32_t>::const_iterator it_i = visitSet.begin();
        it_i != visitSet.end(); ++it_i)
    {
        boost::shared_ptr<const RowType> cols = similarity_.rowItems(*it_i);
        for (RowType::const_iterator it_j = cols->begin();
            it_j != cols->end(); ++it_j)
        {
            uint32_t col = it_j->first;
            assert(col != *it_i && "there should be no similarity value on diagonal line!");
            candidateSet.insert(col);
        }
    }

    // map from candidate item to reason items
    std::map<uint32_t, std::vector<uint32_t> > reasonMap;
    TopItemsQueue queue(howMany);
    for (std::set<uint32_t>::const_iterator it = candidateSet.begin();
        it != candidateSet.end(); ++it)
    {
        uint32_t itemId = *it;
        if(visitSet.find(itemId) == visitSet.end()
           && (!rescorer || !rescorer->isFiltered(itemId)))
        {
            float weight = similarity_.weight(itemId, visitSet, reasonMap[itemId]);
            if(weight > 0)
            {
                queue.insert(QueueItem(itemId, weight));
            }
        }
    }

    recItems.resize(queue.size());
    for(RecommendItemVec::reverse_iterator rit = recItems.rbegin(); rit != recItems.rend(); ++rit)
    {
        QueueItem queueItem = queue.pop();
        rit->itemId_ = queueItem.itemId_;
        rit->weight_ = queueItem.weight_;
        rit->reasonItemIds_.swap(reasonMap[queueItem.itemId_]);
    }
}

void IncrementalItemCF::flush()
{
    covisitation_.dump();
    similarity_.dump();
}

NS_IDMLIB_RESYS_END

