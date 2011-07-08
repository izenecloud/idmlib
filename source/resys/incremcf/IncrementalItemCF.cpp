#include <idmlib/resys/incremcf/IncrementalItemCF.h>

#include <math.h> // for sqrt
#include <algorithm>

#include <glog/logging.h>

namespace
{

/**
 * Calculate similarity value from covisit values.
 */
inline float similarity(
    uint32_t visit_i_j,
    uint32_t visit_i_i,
    uint32_t visit_j_j
)
{
    assert(visit_i_i && visit_j_j);

    return (float)visit_i_j / sqrt(visit_i_i * visit_j_j);
}

}

NS_IDMLIB_RESYS_BEGIN

IncrementalItemCF::IncrementalItemCF(
    const std::string& covisit_path, 
    size_t covisit_row_cache_size,
    const std::string& item_item_similarity_path,
    size_t similarity_row_cache_size, 
    const std::string& item_neighbor_path,
    size_t topK,
    const std::string& user_recommendItem_path,
    size_t max_items_stored_for_each_user
)
    : covisitation_(covisit_path, covisit_row_cache_size)
    , similarity_(item_item_similarity_path,similarity_row_cache_size,item_neighbor_path,topK)
    , userRecommendItems_(user_recommendItem_path)
    , max_items_stored_for_each_user_(max_items_stored_for_each_user)
{
}

IncrementalItemCF::~IncrementalItemCF()
{
}

void IncrementalItemCF::batchBuild()
{
    ///batchBuild is not used currently
    //while(userIterator.hasNext())
    //{
	///build covisit
    //}
    ///build similarity
    ///build neightor
    //userIterator.reset();
    //while(userIterator.hasNext())
    //{
       ///store recommenditems for user
    //}
}

void IncrementalItemCF::buildMatrix(
    std::list<uint32_t>& oldItems,
    std::list<uint32_t>& newItems
)
{
    updateVisitMatrix(oldItems, newItems);

    updateSimMatrix_(oldItems);
}

void IncrementalItemCF::updateVisitMatrix(
    std::list<uint32_t>& oldItems,
    std::list<uint32_t>& newItems
)
{
    covisitation_.visit(oldItems, newItems);
}

void IncrementalItemCF::buildSimMatrix()
{
    typedef ItemCoVisitation<CoVisitFreq>::RowType CoVisitRow;
    typedef SimilarityMatrix<uint32_t,float>::RowType SimRow;

    LOG(INFO) << "start building similarity matrix...";

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

        SimRow simRow;
        for (CoVisitRow::const_iterator it_j = cols.begin();
            it_j != cols.end(); ++it_j)
        {
            const uint32_t col = it_j->first;

            // no similarity value on diagonal line
            if (row == col)
                continue;

            const uint32_t visit_i_j = it_j->second.freq;
            assert(visit_i_j && "the freq value in visit matrix should be positive.");

            uint32_t visit_i_i = 0;
            CoVisitRow::const_iterator findIt = cols.find(row);
            if (findIt != cols.end())
            {
                visit_i_i = findIt->second.freq;
            }
            const uint32_t visit_j_j = covisitation_.coeff(col, col);

            simRow[col] = similarity(visit_i_j, visit_i_i, visit_j_j);
        }

        similarity_.updateRowItems(row, simRow);
        similarity_.loadNeighbor(row);
    }
    std::cout << "\rbuilding row num: " << rowNum << std::endl;

    LOG(INFO) << "finish building similarity matrix";
}

void IncrementalItemCF::updateSimMatrix_(const std::list<uint32_t>& rows)
{
    typedef SimilarityMatrix<uint32_t,float>::RowType RowType;
    std::set<uint32_t> inputSet(rows.begin(), rows.end());
    std::set<uint32_t> colSet;

    for (std::list<uint32_t>::const_iterator it_i = rows.begin();
        it_i != rows.end(); ++it_i)
    {
        uint32_t row = *it_i;

        // phase 1: update columns in each row
        boost::shared_ptr<const RowType> rowItems = similarity_.rowItems(row);
        for (RowType::const_iterator it_j = rowItems->begin();
            it_j != rowItems->end(); ++it_j)
        {
            uint32_t col = it_j->first;
            assert(col != row && "there should be no similarity value on diagonal line!");

            // except those in inputSet as they would be updated in phase 2
            if (inputSet.find(col) == inputSet.end())
            {
                float sim = calcSimValue_(row, col);
                similarity_.coeff(row, col, sim);
                similarity_.coeff(col, row, sim);

                colSet.insert(col);
            }
        }

        // phase 2: update all pairs in rows
        for (std::list<uint32_t>::const_iterator it_j = rows.begin();
            it_j != rows.end(); ++it_j)
        {
            uint32_t col = *it_j;

            if (row == col)
                continue;

            float sim = calcSimValue_(row, col);
            similarity_.coeff(row, col, sim);
        }

        // update neighbor for this row
        similarity_.loadNeighbor(row);
    }

    // update neighbor for col items
    for (std::set<uint32_t>::const_iterator it = colSet.begin();
        it != colSet.end(); ++it)
    {
        similarity_.loadNeighbor(*it);
    }
}

float IncrementalItemCF::calcSimValue_(
    uint32_t row,
    uint32_t col
)
{
    float sim = 0;
    const uint32_t visit_i_j = covisitation_.coeff(row, col);

    if (visit_i_j)
    {
        const uint32_t visit_i_i = covisitation_.coeff(row, row);
        const uint32_t visit_j_j = covisitation_.coeff(col, col);
        sim = similarity(visit_i_j, visit_i_i, visit_j_j);
    }

    return sim;
}

void IncrementalItemCF::buildUserRecommendItems(
    uint32_t userId,
    const std::set<uint32_t>& visitItems,
    ItemRescorer* rescorer
)
{
    typedef SimilarityMatrix<uint32_t,float>::RowType RowType;

    // get candidate items which has similarity value with items
    std::set<uint32_t> candidateSet;
    for (std::set<uint32_t>::const_iterator it_i = visitItems.begin();
        it_i != visitItems.end(); ++it_i)
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

    RecommendItemType recommendItem;
    for (std::set<uint32_t>::const_iterator it = candidateSet.begin();
        it != candidateSet.end(); ++it)
    {
        uint32_t itemId = *it;
        if(visitItems.find(itemId) == visitItems.end()
           && (!rescorer || !rescorer->isFiltered(itemId)))
        {
            float weight = similarity_.weight(itemId, visitItems);
            if(weight > 0)
                recommendItem.push_back(std::make_pair(itemId,weight));
        }
    }
    std::sort(recommendItem.begin(), recommendItem.end(),similarityCompare<ItemType,MeasureType>);

    if(recommendItem.size() > max_items_stored_for_each_user_) 
        recommendItem.resize(max_items_stored_for_each_user_);

    userRecommendItems_.setRecommendItem(userId, recommendItem);
}

void IncrementalItemCF::getTopItems(
    int howMany,
    const std::vector<uint32_t>& itemIds,
    std::list<RecommendedItem>& topItems,
    ItemRescorer* rescorer
)
{
    std::set<uint32_t> filterSet(itemIds.begin(), itemIds.end());
    boost::unordered_map<uint32_t, float> resultSet;
    for(size_t i = 0; i < itemIds.size(); ++i)
    {
        std::vector<std::pair<uint32_t, float> > similarities;
        if(similarity_.itemSimilarity(itemIds[i], similarities))
        {
            std::vector<std::pair<uint32_t, float> >::iterator iter;
            for(iter = similarities.begin(); iter != similarities.end(); ++iter)
            {
                if(filterSet.find(iter->first) == filterSet.end()
                   && (!rescorer || !rescorer->isFiltered(iter->first)))
                {
                    resultSet[iter->first] += iter->second;
                }
            }
        }
    }
    TopItemsQueue resultQueue(howMany);
    boost::unordered_map<uint32_t, float>::iterator it = resultSet.begin();
    for(; it != resultSet.end(); ++it)
    {
        resultQueue.insert(RecommendedItem(it->first, it->second));
    }
    size_t count = resultQueue.size();
    for(size_t i = 0; i < count; ++i)
    {
        RecommendedItem item = resultQueue.pop();
        item.value /= itemIds.size(); ///for normalization
        topItems.push_front(item);
    }
}

void IncrementalItemCF::getTopItems(
    int howMany, 
    uint32_t userId,
    std::list<RecommendedItem>& topItems, 
    ItemRescorer* rescorer
)
{
    RecommendItemType recommendItem;
    if(userRecommendItems_.getRecommendItem(userId, recommendItem))
    {
        int count = 0;
        for(size_t i = 0; i < recommendItem.size(); ++i)
        {
            std::pair<uint32_t, float>& item = recommendItem[i];
            if(!rescorer || !rescorer->isFiltered(item.first))
            {
                topItems.push_back(RecommendedItem(item.first, item.second));
                if(++count >= howMany)
                    break;
            }
        }
    }
}

void IncrementalItemCF::flush()
{
    covisitation_.dump();
    similarity_.dump();
    userRecommendItems_.flush();
}

NS_IDMLIB_RESYS_END

