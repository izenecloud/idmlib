/**
 * @file document_similarity.h
 * @author Zhongxia Li
 * @date Mar 11, 2011
 * @brief Document Similarity
 */
#ifndef DOCUMENT_SIMILARITY_H_
#define DOCUMENT_SIMILARITY_H_

#include <iostream>
#include <string>

#include <boost/shared_ptr.hpp>

#include <idmlib/idm_types.h>

#include <idmlib/semantic_space/explicit_semantic_interpreter.h>
#include <idmlib/semantic_space/term_doc_matrix_defs.h>
#include <idmlib/util/idm_analyzer.h>
#include <idmlib/similarity/document_similarity_index.h>
#include <idmlib/util/collection_file_util.h>
#include <idmlib/util/FSUtil.hpp>
#include <ir/id_manager/IDManager.h>

//#define DOCSIM_TEST

using namespace idmlib::ssp;
using namespace izenelib::ir::idmanager;

NS_IDMLIB_SIM_BEGIN

class DocumentSimilarity
{
public:

	/**
	 * @brief Document similarity
	 * This class is a encapsulation like the mining manager, may be integrated to mining manager
	 * and use indexed data in processing to save disk space and improve performance.
	 * Here the processing is independent of index data, for ESA has provided basic functions for
	 * processing semantic space data.
	 *
	 * @param esasspPath  ESA resource(wiki) path
	 * @param laResPath   LA resource(cma) path
	 * @param colBasePath Collection base path, documents set to be processed
	 * @param colsspPath  Collection data processing path
	 * @param docSimPath  Document similarity index path
	 * @param maxDoc      Max documents of collection to be processed
	 *
	 * @deprecated the performance of SemanticSpace with db storage is low.
	 */
	DocumentSimilarity(
			const std::string& esasspPath,
			const std::string& laResPath,
			const std::string& colBasePath,
			const std::string& colsspPath,
			const std::string& docSimPath,
			weight_t thresholdSim,
			const count_t& maxDoc,
			bool rebuild = false
			)
	: colFileUtil_(new idmlib::util::CollectionFileUtil(colBasePath))
	{
#ifdef SSP_TIME_CHECKER
	    idmlib::util::TimeChecker timer("DocumentSimilarity Init");
#endif

		// Explicit semantic interpreter initialized with wiki knowledge
		boost::shared_ptr<SemanticSpace> pWikiESSpace(new ExplicitSemanticSpace(esasspPath, SemanticSpace::LOAD));
#ifdef DOCSIM_TEST
		//pWikiESSpace->Print();
#endif
		pEsaInterpreter_.reset(new ExplicitSemanticInterpreter(pWikiESSpace));

		// create or load pre-processing data of collection (documents set)
		// can use indexed data..
		if (rebuild) {
#ifdef SSP_TIME_CHECKER
		    idmlib::util::TimeChecker timer("Pre Process Collection(documents)");
#endif
			pDocVecSpace_.reset(new DocumentVectorSpace(colsspPath, SemanticSpace::CREATE));
			boost::shared_ptr<SemanticSpaceBuilder> pCollectionBuilder(
					new SemanticSpaceBuilder(pDocVecSpace_, laResPath, colBasePath, maxDoc));
			pCollectionBuilder->Build();
		}
		else {
			pDocVecSpace_.reset(new DocumentVectorSpace(colsspPath, SemanticSpace::LOAD));
		}
#if 0
		pDocVecSpace_->Print();
#endif

		// similarity index
		pDocSimIndex_.reset(new DocumentSimilarityIndex(docSimPath, thresholdSim));
	}

    DocumentSimilarity(
            const std::string& wikiIndexDir,
            const std::string& laResPath,
            const std::string& colBasePath,
            const std::string& docSimPath,
            weight_t thresholdSim,
            const count_t& maxDoc
            )
    : colFileUtil_(new idmlib::util::CollectionFileUtil(colBasePath))
    , maxDoc_(maxDoc)
    {
        pWikiIndexer_ = idmlib::ssp::IzeneIndexHelper::createIndexer(wikiIndexDir);
        pEsaInterpreter_.reset(new SemanticInterpreter(pWikiIndexer_, laResPath, colBasePath));

        encoding_ = izenelib::util::UString::UTF_8;

        string idDir = colFileUtil_->getCollectionDataDir() + "/id";
        if (!exists(idDir)) {
            DLOG(ERROR) << "id directory dose not exsited: " <<idDir << std::endl;;
        }
        pIdManager_.reset(new IDManager(idDir));
        BOOST_ASSERT(pIdManager_);

        // similarity index
        pDocSimIndex_.reset(new DocumentSimilarityIndex(docSimPath, thresholdSim));
    }

	~DocumentSimilarity()
	{
	}

public:

	/**
	 * @deprecated
	 */
	void DoSim();

	bool computeSimilarity();

private:
	void processDocument_(SCDDocPtr& pDoc);

private:
	boost::shared_ptr<SemanticInterpreter> pEsaInterpreter_;
	boost::shared_ptr<Indexer> pWikiIndexer_;
	boost::shared_ptr<IDManager> pIdManager_;

	izenelib::util::UString::EncodingType encoding_;
	count_t maxDoc_;

	typedef std::vector<std::pair<izenelib::util::UString, izenelib::util::UString> >::iterator doc_properties_iterator;

	// old
	boost::shared_ptr<SemanticSpace> pDocVecSpace_; // processed collection data, xxx

	boost::shared_ptr<DocumentSimilarityIndex> pDocSimIndex_;

	boost::shared_ptr<idmlib::util::CollectionFileUtil> colFileUtil_;

};

NS_IDMLIB_SIM_END

#endif /* DOCUMENT_SIMILARITY_H_ */
