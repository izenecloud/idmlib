/**
 * @file WikiIndexBuilder.h
 * @author Zhongxia Li
 * @date Jun 1, 2011
 * @brief Building weighted inverted index of Wikipedia for explicit semantic analysis.
 */
#ifndef WIKI_INDEX_BUILDER_H_
#define WIKI_INDEX_BUILDER_H_

#include <boost/shared_ptr.hpp>

#include "WikiIndex.h"
#include "MemWikiIndex.h"
#include "IzeneWikiIndex.h"
#include <idmlib/idm_types.h>
#include <idmlib/util/CollectionUtil.h>


using namespace idmlib::util;

NS_IDMLIB_SSP_BEGIN

class WikiIndexBuilder : public CollectionProcessor
{
public:
    WikiIndexBuilder(
        const std::string& wikiColBasePath,
        const std::string& laResPath,
        const std::vector<std::string>& processProperties,
        size_t maxDoc = 0,
        bool removeStopwords = false,
        izenelib::util::UString::EncodingType encoding = izenelib::util::UString::UTF_8)
            : CollectionProcessor(wikiColBasePath, laResPath, processProperties, maxDoc, removeStopwords, encoding)
    {
    }

public:
    void build(boost::shared_ptr<WikiIndex> wikiIndex)
    {
        wikiIndex_ = wikiIndex;

        processSCD();
    }

private:
    /*virtual */
    void processDocumentAnalyzedContent()
    {
        wikiDoc_.docid = curDocId_;
        wikiDoc_.pTermIdList = pTermIdList_;

        wikiIndex_->insertDocument(wikiDoc_);
    }

    /*virtual */
    void postProcess()
    {
        wikiIndex_->postProcess();
    }

private:
    boost::shared_ptr<WikiIndex> wikiIndex_;
    WikiDoc wikiDoc_;
};


class IzeneWikiIndexBuilder : public CollectionProcessor
{
public:
    IzeneWikiIndexBuilder(
        const std::string& wikiColBasePath,
        const std::string& laResPath,
        const std::vector<std::string>& processProperties,
        size_t maxDoc = 0,
        izenelib::util::UString::EncodingType encoding = izenelib::util::UString::UTF_8)
            : CollectionProcessor(wikiColBasePath, laResPath, processProperties, maxDoc, encoding)
    {
        wikiIndex_.reset(new IzeneWikiIndex(pIdmAnalyzer_, encoding_));
    }

public:
    void build()
    {
        processSCD();
    }

private:
    /*virtual */
    void processDocument()
    {
        wikiIndex_->insertDocument(curSCDDoc_);
    }

    /*virtual */
    void postProcess()
    {
        wikiIndex_->postProcess();
    }

private:
    boost::shared_ptr<IzeneWikiIndex> wikiIndex_;
    WikiDoc wikiDoc_;
};

NS_IDMLIB_SSP_END

#endif /* WIKI_INDEX_BUILDER_H_ */
