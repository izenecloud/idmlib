/**
 * @file IzeneWikiIndex.h
 * @author Zhongxia Li
 * @date Jun 3, 2011
 * @brief Use izenelib/ir/index_manger to build inverted index for Wikipedia
 */
#ifndef IZENE_WIKI_INDEX_H_
#define IZENE_WIKI_INDEX_H_

#include <idmlib/idm_types.h>

#include "WikiIndex.h"
#include "izene_index_helper.h"
#include <idmlib/util/CollectionUtil.h>

#include <ir/index_manager/index/IndexerDocument.h>
#include <ir/index_manager/index/IndexReader.h>

NS_IDMLIB_SSP_BEGIN

/**
 * Wrap up izenelib indexer for building wiki index
 * @deprecated
 * @brief For performance consideration, this indexer is not suitable for ESA,
 * and so will not be used currently practically (MemWikiIndex is in use).
 * 1. izenelib indexer has no cache mechanism for read currently.
 * 2. wiki index need to store term-concept weights to speed up ESA computation.
 */
class IzeneWikiIndex : public WikiIndex
{
public:
    IzeneWikiIndex(
        boost::shared_ptr<idmlib::util::IDMAnalyzer> pIdmAnalyzer,
        boost::shared_ptr<IDManager> pIdManager,
        izenelib::util::UString::EncodingType encoding,
        std::string wikiIndexDir = "./esa/wiki"
    )
            : WikiIndex(wikiIndexDir)
            , pIdmAnalyzer_(pIdmAnalyzer)
            , pIdManager_(pIdManager)
            , encoding_(encoding)
    {
        pIndexer_ = idmlib::ssp::IzeneIndexHelper::createIndexer(wikiIndexDir+"/izene_index");
        laInput_.reset(new TermIdList());
    }

public:
    void insertDocument(SCDDocPtr& SCDDoc)
    {
        IndexerDocument indexDocument;
        bool ret = prepareDocument_(SCDDoc, indexDocument);

        if ( !ret)
            return;

        pIndexer_->insertDocument(indexDocument);
    }

    /*virtual*/
    void insertDocument(WikiDoc& wikiDoc)
    {
    }

    /*virtual*/
    void postProcess()
    {
    }

    /*virtual*/
    void load()
    {
        // todo add cache for izenelib indexer
    }

public:
    bool prepareDocument_(SCDDocPtr& pDoc, IndexerDocument& indexDocument)
    {
        uint32_t docId = 0;

        CollectionProcessor::doc_properties_iterator proIter;
        for (proIter = pDoc->begin(); proIter != pDoc->end(); proIter ++)
        {
            izenelib::util::UString propertyName = proIter->first;
            const izenelib::util::UString & propertyValue = proIter->second;
            propertyName.toLowerString();

            // see IzeneIndexHelper::setIndexConfig()
            IndexerPropertyConfig indexerPropertyConfig;
            indexerPropertyConfig.setIsMultiValue(false);

            if (propertyName == izenelib::util::UString("docid", encoding_))
            {
                indexerPropertyConfig.setPropertyId(idmlib::ssp::IzeneIndexHelper::getPropertyIdByName("DOCID"));
                indexerPropertyConfig.setName("DOCID");
                indexerPropertyConfig.setIsIndex(false);
                indexerPropertyConfig.setIsFilter(false);
                indexerPropertyConfig.setIsAnalyzed(false);
                indexerPropertyConfig.setIsStoreDocLen(false);

                bool ret = pIdManager_->getDocIdByDocName(propertyValue, docId, false);
                if (ret)  /*exist*/;
                indexDocument.setId(0);
                indexDocument.setDocId(docId, idmlib::ssp::IzeneIndexHelper::COLLECTION_ID);
            }
            else if (propertyName == izenelib::util::UString("date", encoding_) )
            {
                indexerPropertyConfig.setPropertyId(idmlib::ssp::IzeneIndexHelper::getPropertyIdByName("DATE"));
                indexerPropertyConfig.setName("DATE");
                indexerPropertyConfig.setIsIndex(true);
                indexerPropertyConfig.setIsFilter(true);
                indexerPropertyConfig.setIsAnalyzed(false);
                indexerPropertyConfig.setIsStoreDocLen(false);
                struct tm atm;
                int64_t time = mktime(&atm);
                indexDocument.insertProperty(indexerPropertyConfig, time);
            }
            else
            {
                if ( propertyName == izenelib::util::UString("title", encoding_) )
                {
                    indexerPropertyConfig.setPropertyId(idmlib::ssp::IzeneIndexHelper::getPropertyIdByName("Title"));
                    indexerPropertyConfig.setName("Title");
                }
                else if ( propertyName == izenelib::util::UString("content", encoding_))
                {
                    indexerPropertyConfig.setPropertyId(idmlib::ssp::IzeneIndexHelper::getPropertyIdByName("Content"));
                    indexerPropertyConfig.setName("Content");
                }
                else
                {
                    continue;
                }

                // "Title" or "Content"
                indexerPropertyConfig.setIsIndex(true);
                indexerPropertyConfig.setIsAnalyzed(true);
                indexerPropertyConfig.setIsFilter(false);
                indexerPropertyConfig.setIsStoreDocLen(true);

                laInput_->resize(0);
                laInput_->setDocId(docId); // <DOCID> property have to come first in SCD
                pIdmAnalyzer_->GetTermIdList(pIdManager_.get(), propertyValue, *laInput_);
                indexDocument.insertProperty(indexerPropertyConfig, laInput_);
            }
        }

        return true;
    }

private:
    boost::shared_ptr<idmlib::util::IDMAnalyzer> pIdmAnalyzer_;
    boost::shared_ptr<IDManager> pIdManager_;

    izenelib::util::UString::EncodingType encoding_;

    boost::shared_ptr<Indexer> pIndexer_;
    boost::shared_ptr<TermIdList> laInput_;
};

NS_IDMLIB_SSP_END

#endif /* IZENEWIKIINDEX_H_ */
