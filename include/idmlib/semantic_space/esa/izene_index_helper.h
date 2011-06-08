/**
 * @file izene_index_helper.h
 * @author Zhongxia Li
 * @date May 13, 2011
 * @brief 
 */
#ifndef IZENE_INDEX_HELPER_H_
#define IZENE_INDEX_HELPER_H_

#include <idmlib/idm_types.h>

#include <ir/index_manager/index/Indexer.h>
#include <ir/index_manager/utility/StringUtils.h>
#include <ir/index_manager/utility/IndexManagerConfig.h>

using namespace izenelib::ir::indexmanager;

NS_IDMLIB_SSP_BEGIN

class IzeneIndexHelper
{
public:
    /**
     * Create default indexer for Chinese Wiki corpus
     * @param indexDir
     * @param collectionName
     * @return Indexer
     */
    static boost::shared_ptr<Indexer> createIndexer(const std::string& indexDir, std::string collectionName = "ChnWiki")
    {
        boost::shared_ptr<Indexer> indexer(new Indexer());

        if (indexer)
        {
            IndexManagerConfig config;
            setIndexConfig(config, collectionName, indexDir);

            std::map<std::string, unsigned int> collectionIdMapping;
            collectionIdMapping[collectionName] = COLLECTION_ID;

            indexer->setIndexManagerConfig(config, collectionIdMapping);
        }

        return indexer;
    }

    const static unsigned int COLLECTION_ID = 1;


    static unsigned int getPropertyIdByName(std::string property)
    {
        if (property == "Content")
        {
            return 1;
        }
        else if (property == "DATE")
        {
            return 2;
        }
        else if (property == "DOCID")
        {
            return 3;
        }
        else if (property == "Title")
        {
            return 4;
        }

        return 0;
    }

private:

    static void setIndexConfig(IndexManagerConfig& config, std::string& collectionName, const std::string& indexDir)
    {
        // set as default configuration
        if (!boost::filesystem::exists(indexDir)) {
            boost::filesystem::create_directories(indexDir);
        }
        config.indexStrategy_.indexLocation_ = indexDir;
        config.indexStrategy_.memory_ = 128000000;
        config.indexStrategy_.indexDocLength_ = true;
        config.indexStrategy_.skipInterval_ = 8;
        config.indexStrategy_.maxSkipLevel_ = 3;
        config.indexStrategy_.indexMode_ = "default";
        config.storeStrategy_.param_ = "file";
        config.mergeStrategy_.param_ = "dbt";
        config.indexStrategy_.indexLocations_.push_back("default-collection-dir");

        IndexerCollectionMeta indexCollectionMeta;
        indexCollectionMeta.setName(collectionName);
        // ordered by property names
        indexCollectionMeta.addPropertyConfig( makeIndexPropertyConfig(1, "Content", true, true) );
        indexCollectionMeta.addPropertyConfig( makeIndexPropertyConfig(2, "DATE", true, false, true, false) );
        indexCollectionMeta.addPropertyConfig( makeIndexPropertyConfig(3, "DOCID", false, false, false, false) );
        indexCollectionMeta.addPropertyConfig( makeIndexPropertyConfig(4, "Title", true, true) );

        config.addCollectionMeta(indexCollectionMeta);
    }

    static IndexerPropertyConfig makeIndexPropertyConfig(
            unsigned int propertyid, std::string propertyname, bool index, bool analyzed,
            bool filter=false, bool storeDocLen=true)
    {
        IndexerPropertyConfig indexerPropertyConfig(propertyid, propertyname, index, analyzed);
        indexerPropertyConfig.setIsFilter(filter);
        indexerPropertyConfig.setIsMultiValue(false);
        indexerPropertyConfig.setIsStoreDocLen(storeDocLen);

        return indexerPropertyConfig;
    }
};

NS_IDMLIB_SSP_END

#endif /* IZENE_INDEX_HELPER_H_ */
