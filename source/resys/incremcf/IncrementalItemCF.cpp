#include <idmlib/resys/incremcf/IncrementalItemCF.h>
#include <util/PriorityQueue.h>

#include <map>
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

IncrementalItemCF::UpdateSimFunc::UpdateSimFunc(
    SimMatrix& simMatrix,
    SimNeighbor& simNeighbor,
    const std::list<uint32_t>& excludeRowList
)
    : simMatrix_(simMatrix)
    , simNeighbor_(simNeighbor)
    , excludeRows_(excludeRowList.begin(), excludeRowList.end())
{
}

void IncrementalItemCF::UpdateSimFunc::operator()(
    uint32_t row,
    uint32_t col,
    float sim
)
{
    if (excludeRows_.find(row) == excludeRows_.end())
    {
        simMatrix_.update_elem(row, col, sim);
        updatedRows_.insert(row);
    }
}

void IncrementalItemCF::UpdateSimFunc::updateNeighbor()
{
    for (std::set<uint32_t>::const_iterator it = updatedRows_.begin();
        it != updatedRows_.end(); ++it)
    {
        uint32_t row = *it;
        boost::shared_ptr<const SimRow> simRow = simMatrix_.row(row);
        simNeighbor_.updateNeighbor(row, *simRow);
    }
}

IncrementalItemCF::IncrementalItemCF(
    const ItemCFParam& itemCFParam,
    MatrixSharder* matrixSharder
)
    : covisitation_(itemCFParam.coVisitPath, itemCFParam.coVisitCacheSize, matrixSharder)
    , visitMatrix_(covisitation_.matrix())
    , simMatrix_(itemCFParam.simMatrixCacheSize, itemCFParam.simMatrixPath)
    , simNeighbor_(itemCFParam.simNeighborPath, itemCFParam.simNeighborTopK)
    , visitFreqDB_(itemCFParam.visitFreqPath)
    , matrixSharder_(matrixSharder)
{
    visitFreqDB_.open();
}

IncrementalItemCF::~IncrementalItemCF()
{
}

void IncrementalItemCF::flush()
{
    covisitation_.flush();
    simMatrix_.flush();
    simNeighbor_.flush();
    visitFreqDB_.flush();
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

void IncrementalItemCF::updateMatrix(
    const std::list<uint32_t>& oldItems,
    const std::list<uint32_t>& newItems)
{
    updateVisitMatrix(oldItems, newItems);

    if (matrixSharder_)
    {
        updateSimMatrix_(newItems);
        updateSimMatrix_(oldItems, newItems);
    }
    else
    {
        updateSymmetricMatrix_(newItems);
    }
}

void IncrementalItemCF::updateVisitMatrix(
    const std::list<uint32_t>& oldItems,
    const std::list<uint32_t>& newItems)
{
    covisitation_.visit(oldItems, newItems);

    updateVisitFreq_(newItems);
}

void IncrementalItemCF::updateVisitFreq_(const std::list<uint32_t>& newItems)
{
    for (std::list<uint32_t>::const_iterator it = newItems.begin();
        it != newItems.end(); ++it)
    {
        uint32_t freq = 0;
        visitFreqDB_.get(*it, freq);
        ++freq;
        visitFreqDB_.update(*it, freq);
    }
}

void IncrementalItemCF::updateSimMatrix_(const std::list<uint32_t>& rows)
{
    for (std::list<uint32_t>::const_iterator it = rows.begin();
        it != rows.end(); ++it)
    {
        uint32_t row = *it;

        if (! isMyRow_(row))
            continue;

        boost::shared_ptr<const VisitRow> coVisitRow = visitMatrix_.row(row);
        updateSimRow_(row, *coVisitRow);
    }
}

void IncrementalItemCF::updateSimMatrix_(
    const std::list<uint32_t>& rows,
    const std::list<uint32_t>& cols
)
{
    for (std::list<uint32_t>::const_iterator it_i = rows.begin();
        it_i != rows.end(); ++it_i)
    {
        uint32_t row = *it_i;

        if (! isMyRow_(row))
            continue;

        boost::shared_ptr<const VisitRow> coVisitRow = visitMatrix_.row(row);
        VisitRow updateRow;
        getCoVisitFreq_(*coVisitRow, row, updateRow[row].freq);

        for (std::list<uint32_t>::const_iterator it_j = cols.begin();
            it_j != cols.end(); ++it_j)
        {
            uint32_t col = *it_j;
            getCoVisitFreq_(*coVisitRow, col, updateRow[col].freq);
        }

        updateSimRow_(row, updateRow);
    }
}

void IncrementalItemCF::updateSymmetricMatrix_(const std::list<uint32_t>& rows)
{
    UpdateSimFunc func(simMatrix_, simNeighbor_, rows);

    for (std::list<uint32_t>::const_iterator it = rows.begin();
        it != rows.end(); ++it)
    {
        uint32_t row = *it;

        if (! isMyRow_(row))
            continue;

        boost::shared_ptr<const VisitRow> coVisitRow = visitMatrix_.row(row);
        updateSimRow_(row, *coVisitRow, &func);
    }

    func.updateNeighbor();
}

void IncrementalItemCF::updateSimRow_(
    uint32_t row,
    const VisitRow& coVisitRow,
    UpdateSimFunc* func
)
{
    uint32_t visit_i_i = 0;
    getCoVisitFreq_(coVisitRow, row, visit_i_i);

    boost::shared_ptr<SimRow> simRow(new SimRow);
    for (VisitRow::const_iterator it_j = coVisitRow.begin();
        it_j != coVisitRow.end(); ++it_j)
    {
        uint32_t col = it_j->first;

        // no similarity value on diagonal line
        if (row == col)
            continue;

        uint32_t visit_i_j = it_j->second.freq;
        uint32_t visit_j_j = 0;
        visitFreqDB_.getValue(col, visit_j_j);
        assert(visit_i_j && visit_i_i && visit_j_j &&
               "the freq value in visit matrix should be positive.");

        float sim = (float)visit_i_j / sqrt(visit_i_i * visit_j_j);
        (*simRow)[col] = sim;

        if (func)
        {
            (*func)(col, row, sim);
        }
    }

    simMatrix_.update_row(row, simRow);
    simNeighbor_.updateNeighbor(row, *simRow);
}

void IncrementalItemCF::buildSimMatrix()
{
    LOG(INFO) << "start building similarity matrix...";

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

        uint32_t row = it_i->first;
        const VisitRow& cols = it_i->second;
        elemNum += cols.size();

        updateSimRow_(row, cols);
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

void IncrementalItemCF::recommend(
    int howMany,
    const std::vector<uint32_t>& visitItems,
    RecommendItemVec& recItems,
    const ItemRescorer* rescorer
)
{
    std::set<uint32_t> candidateSet;
    getCandidateSet(visitItems, candidateSet);

    recommendFromCandidateSet(howMany, visitItems, candidateSet,
                              recItems, rescorer);
}

void IncrementalItemCF::recommend(
    int howMany,
    const ItemWeightMap& visitItemWeights,
    RecommendItemVec& recItems,
    const ItemRescorer* rescorer
)
{
    std::set<uint32_t> candidateSet;
    getCandidateSet(visitItemWeights, candidateSet);

    recommendFromCandidateSet(howMany, visitItemWeights, candidateSet,
                              recItems, rescorer);
}

void IncrementalItemCF::getCandidateSet(
    const std::vector<uint32_t>& visitItems,
    std::set<uint32_t>& candidateSet
)
{
    std::vector<uint32_t>::const_iterator itEnd = visitItems.end();

    for (std::vector<uint32_t>::const_iterator it = visitItems.begin();
        it != itEnd; ++it)
    {
        if (isMyRow_(*it))
        {
            simNeighbor_.getNeighborSet(*it, candidateSet);
        }
    }
}

void IncrementalItemCF::getCandidateSet(
    const ItemWeightMap& visitItemWeights,
    std::set<uint32_t>& candidateSet
)
{
    ItemWeightMap::const_iterator itEnd = visitItemWeights.end();

    for (ItemWeightMap::const_iterator it = visitItemWeights.begin();
        it != itEnd; ++it)
    {
        uint32_t itemId = it->first;

        if (isMyRow_(itemId) && it->second > 0)
        {
            simNeighbor_.getNeighborSet(itemId, candidateSet);
        }
    }
}

void IncrementalItemCF::recommendFromCandidateSet(
    int howMany,
    const std::vector<uint32_t>& visitItems,
    const std::set<uint32_t>& candidateSet,
    RecommendItemVec& recItems,
    const ItemRescorer* rescorer
)
{
    ItemReasonMap reasonMap;
    TopItemsQueue queue(howMany);
    std::set<uint32_t> visitSet(visitItems.begin(), visitItems.end());
    std::set<uint32_t>::const_iterator visitSetEnd = visitSet.end();

    for (std::set<uint32_t>::const_iterator it = candidateSet.begin();
        it != candidateSet.end(); ++it)
    {
        uint32_t itemId = *it;

        if (isMyRow_(itemId) && visitSet.find(itemId) == visitSetEnd &&
            !(rescorer && rescorer->isFiltered(itemId)))
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

void IncrementalItemCF::recommendFromCandidateSet(
    int howMany,
    const ItemWeightMap& visitItemWeights,
    const std::set<uint32_t>& candidateSet,
    RecommendItemVec& recItems,
    const ItemRescorer* rescorer
)
{
    ItemReasonMap reasonMap;
    TopItemsQueue queue(howMany);
    ItemWeightMap::const_iterator visitWeightsEnd = visitItemWeights.end();

    for (std::set<uint32_t>::const_iterator it = candidateSet.begin();
        it != candidateSet.end(); ++it)
    {
        uint32_t itemId = *it;

        if (isMyRow_(itemId) && visitItemWeights.find(itemId) == visitWeightsEnd &&
            !(rescorer && rescorer->isFiltered(itemId)))
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

NS_IDMLIB_RESYS_END
