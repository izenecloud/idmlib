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
        : itemId_(itemId)
        , weight_(weight)
    {}

    uint32_t itemId_;
    float weight_;
};

class TopItemsQueue
    : public izenelib::util::PriorityQueue<QueueItem>
{
public:
    TopItemsQueue(size_t size)
    {
        this->initialize(size);
    }

protected:
    bool lessThan(const QueueItem& o1, const QueueItem& o2) const
    {
        return (o1.weight_ < o2.weight_);
    }
};

// map from candidate item to reason items
typedef std::map<uint32_t, std::vector<uint32_t> > ItemReasonMap;

void getRecommendItemFromQueue(
        TopItemsQueue& queue,
        ItemReasonMap& reasonMap,
        idmlib::recommender::RecommendItemVec& recItems)
{
    recItems.resize(queue.size());

    for (idmlib::recommender::RecommendItemVec::reverse_iterator rit = recItems.rbegin();
            rit != recItems.rend(); ++rit)
    {
        QueueItem queueItem = queue.pop();
        rit->itemId_ = queueItem.itemId_;
        rit->weight_ = queueItem.weight_;
        rit->reasonItemIds_.swap(reasonMap[queueItem.itemId_]);
    }
}

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
    , visitMatrix_(covisitation_.matrix())
    , simMatrix_(similarity_row_cache_size, item_item_similarity_path)
    , simNeighbor_(item_neighbor_path, topK)
{
}

IncrementalItemCF::~IncrementalItemCF()
{
}

void IncrementalItemCF::updateMatrix(
    const std::list<uint32_t>& oldItems,
    const std::list<uint32_t>& newItems)
{
    updateVisitMatrix(oldItems, newItems);

    updateSimMatrix_(newItems);
}

void IncrementalItemCF::updateVisitMatrix(
    const std::list<uint32_t>& oldItems,
    const std::list<uint32_t>& newItems)
{
    covisitation_.visit(oldItems, newItems);
}

void IncrementalItemCF::buildSimMatrix()
{
    LOG(INFO) << "start building similarity matrix...";

    // empty set used to call updateSimRow_()
    std::set<uint32_t> rowSet;
    std::set<uint32_t> colSet;

    uint64_t rowNum = 0;
    uint64_t elemNum = 0;
    for (VisitMatrix::iterator it_i = visitMatrix_.begin();
            it_i != visitMatrix_.end(); ++it_i)
    {
        if (++rowNum % 1000 == 0)
        {
            std::cout << "\rbuilding row[" << rowNum << "], "
                      << *this << std::flush;
        }

        const uint32_t row = it_i->first;
        const VisitRow& cols = it_i->second;
        elemNum += cols.size();

        updateSimRow_(row, cols, false, rowSet, colSet);
    }
    std::cout << "\rbuilding row[" << rowNum << "], "
              << *this << std::endl;

    float density = 0;
    if (rowNum)
    {
        density = (float)elemNum / (rowNum*rowNum) * 100;
    }
    LOG(INFO) << "covisit matrix element num: " << elemNum;
    LOG(INFO) << "covisit matrix density: " << density << "%";
    LOG(INFO) << "finish building similarity matrix";
}

void IncrementalItemCF::updateSimRow_(
    uint32_t row,
    const VisitRow& coVisitRow,
    bool isUpdateCol,
    const std::set<uint32_t>& rowSet,
    std::set<uint32_t>& colSet
)
{
    uint32_t visit_i_i = 0;
    VisitRow::const_iterator findIt = coVisitRow.find(row);
    if (findIt != coVisitRow.end())
    {
        visit_i_i = findIt->second.freq;
    }

    boost::shared_ptr<SimRow> simRow(new SimRow);
    for (VisitRow::const_iterator it_j = coVisitRow.begin();
        it_j != coVisitRow.end(); ++it_j)
    {
        const uint32_t col = it_j->first;

        // no similarity value on diagonal line
        if (row == col)
            continue;

        const uint32_t visit_i_j = it_j->second.freq;
        const uint32_t visit_j_j = visitMatrix_.elem(col, col).freq;
        assert(visit_i_j && visit_i_i && visit_j_j && "the freq value in visit matrix should be positive.");

        float sim = (float)visit_i_j / sqrt(visit_i_i * visit_j_j);
        (*simRow)[col] = sim;

        // also update (col, row) if col does not exist in rowSet
        if (isUpdateCol && rowSet.find(col) == rowSet.end())
        {
            simMatrix_.update_elem(col, row, sim);
            colSet.insert(col);
        }
    }

    simMatrix_.update_row(row, simRow);
    simNeighbor_.updateNeighbor(row, *simRow);
}

void IncrementalItemCF::updateSimMatrix_(const std::list<uint32_t>& rows)
{
    std::set<uint32_t> rowSet(rows.begin(), rows.end());
    std::set<uint32_t> colSet;

    for (std::list<uint32_t>::const_iterator it_i = rows.begin();
        it_i != rows.end(); ++it_i)
    {
        uint32_t row = *it_i;
        boost::shared_ptr<const VisitRow> cols = visitMatrix_.row(row);

        updateSimRow_(row, *cols, true, rowSet, colSet);
    }

    // update neighbor for col items
    for (std::set<uint32_t>::const_iterator it = colSet.begin();
        it != colSet.end(); ++it)
    {
        uint32_t row = *it;
        boost::shared_ptr<const SimRow> simRow = simMatrix_.row(row);
        simNeighbor_.updateNeighbor(row, *simRow);
    }
}

void IncrementalItemCF::recommend(
    int howMany,
    const std::vector<uint32_t>& visitItems,
    RecommendItemVec& recItems,
    ItemRescorer* rescorer
)
{
    std::set<uint32_t> visitSet(visitItems.begin(), visitItems.end());
    std::set<uint32_t>::const_iterator visitSetEnd = visitSet.end();

    // get candidate items which has similarity value with items
    std::set<uint32_t> candidateSet;
    for (std::set<uint32_t>::const_iterator it_i = visitSet.begin();
            it_i != visitSetEnd; ++it_i)
    {
        simNeighbor_.getNeighborSet(*it_i, candidateSet);
    }

    ItemReasonMap reasonMap;
    TopItemsQueue queue(howMany);
    for (std::set<uint32_t>::const_iterator it = candidateSet.begin();
            it != candidateSet.end(); ++it)
    {
        uint32_t itemId = *it;
        if (visitSet.find(itemId) == visitSetEnd
                && !(rescorer && rescorer->isFiltered(itemId)))
        {
            float weight = simNeighbor_.weight(itemId, visitSet, reasonMap[itemId]);
            if (weight > 0)
            {
                queue.insert(QueueItem(itemId, weight));
            }
        }
    }

    getRecommendItemFromQueue(queue, reasonMap, recItems);
}

void IncrementalItemCF::recommend(
    int howMany,
    const ItemWeightMap& visitItemWeights,
    RecommendItemVec& recItems,
    ItemRescorer* rescorer
)
{
    // get candidate items which has positive weight and similarity value
    std::set<uint32_t> candidateSet;
    ItemWeightMap::const_iterator visitWeightsEnd = visitItemWeights.end();
    for (ItemWeightMap::const_iterator it_i = visitItemWeights.begin();
        it_i != visitWeightsEnd; ++it_i)
    {
        if (it_i->second > 0)
        {
            simNeighbor_.getNeighborSet(it_i->first, candidateSet);
        }
    }

    ItemReasonMap reasonMap;
    TopItemsQueue queue(howMany);
    for (std::set<uint32_t>::const_iterator it = candidateSet.begin();
        it != candidateSet.end(); ++it)
    {
        uint32_t itemId = *it;
        if (visitItemWeights.find(itemId) == visitWeightsEnd
                && !(rescorer && rescorer->isFiltered(itemId)))
        {
            float weight = simNeighbor_.weight(itemId, visitItemWeights, reasonMap[itemId]);
            if (weight > 0)
            {
                queue.insert(QueueItem(itemId, weight));
            }
        }
    }

    getRecommendItemFromQueue(queue, reasonMap, recItems);
}

void IncrementalItemCF::flush()
{
    covisitation_.flush();
    simMatrix_.flush();
    simNeighbor_.flush();
}

void IncrementalItemCF::print(std::ostream& ostream) const
{
    ostream << "CoVisit " << visitMatrix_
            << "Similarity " << simMatrix_
            << simNeighbor_;
}

std::ostream& operator<<(std::ostream& out, const IncrementalItemCF& increItemCF)
{
    increItemCF.print(out);
    return out;
}

NS_IDMLIB_RESYS_END
