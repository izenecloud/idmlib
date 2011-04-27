#ifndef IDMLIB_RESYS_SIMILARITY_MATRIX_H
#define IDMLIB_RESYS_SIMILARITY_MATRIX_H

#include <idmlib/idm_types.h>

#include <cache/IzeneCache.h>
#include <util/Int2String.h>
#include <am/beansdb/Hash.h>

#include <ext/hash_map>
#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/hash_map.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include <string>
#include <vector>

NS_IDMLIB_RESYS_BEGIN

using izenelib::util::Int2String;

template<typename ItemType = uint32_t, typename MeasureType = double>
class SimilarityMatrix
{
    typedef __gnu_cxx::hash_map<ItemType, MeasureType> HashType;
    typedef izenelib::cache::IzeneCache<
    ItemType,
    boost::shared_ptr<HashType >,
    izenelib::util::ReadWriteLock,
    izenelib::cache::RDE_HASH,
    izenelib::cache::LFU
    > RowCacheType;

    typedef izenelib::cache::IzeneCache<
    ItemType,
    std::vector<std::pair<ItemType, MeasureType> >,
    izenelib::util::ReadWriteLock,
    izenelib::cache::RDE_HASH,
    izenelib::cache::LFU
    > TopSimilarCacheType;

public:
    SimilarityMatrix(
          const std::string& homePath, 
          size_t row_cache_size, 
          size_t similar_items_cache_size,
          size_t topK = 30
          )
        : store_(homePath)
        , row_cache_(row_cache_size)
        , top_similar_item_cache_(similar_items_cache_size)
        , topK_(topK)
    {
    }

    ~SimilarityMatrix()
    {
        store_.flush();
        store_.close();
    }


    void itemSimilarities(
            ItemType itemId, 
            std::vector<ItemType>& itemIds, 
            std::vector<std::pair<ItemType, double> >& similarities
    )
    {
        typename std::vector<ItemType>::iterator iter= itemIds.begin();
        similarities.resize(itemIds.size());
        for( ; iter != itemIds.end(); ++iter)
        {
            double measure = (double)coeff(itemId,*iter);
            similarities.push_back(std::make_pair(*iter, measure));
        }
    }

    MeasureType coeff(ItemType row, ItemType col)
    {
        boost::shared_ptr<HashType > rowdata;
        if (!row_cache_.getValueNoInsert(row, rowdata))
        {
            rowdata = loadRow(row);
            row_cache_.insertValue(row, rowdata);
        }
        typename HashType::iterator it = rowdata->find(col); 
        if(it == rowdata->end()) return 0;
        return it->second;
    }

    void coeff(ItemType row, ItemType col, MeasureType measure)
    {
	boost::shared_ptr<HashType > rowdata;
	if (!row_cache_.getValueNoInsert(row, rowdata))
	{
            rowdata = loadRow(row);
            row_cache_.insertValue(row, rowdata);
	}
	(*rowdata)[col] = measure;
	saveRow(row,rowdata);
    }

    void gc()
    {
        store_.optimize();
    }

private:

    boost::shared_ptr<HashType > 
    loadRow(ItemType row)
    {
        boost::shared_ptr<HashType > rowdata(new HashType);
        Int2String rowKey(row);
        store_.get(rowKey, *rowdata);
        return rowdata;
    }

    void saveRow(ItemType row,  boost::shared_ptr<HashType > rowdata)
    {
        Int2String rowKey(row);
        store_.insert(rowKey, *rowdata);
    }


private:
    izenelib::am::beansdb::Hash<Int2String, HashType > store_;
    RowCacheType row_cache_;
    TopSimilarCacheType top_similar_item_cache_;
    size_t topK_;
};


NS_IDMLIB_RESYS_END

#endif

