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
#include <idmlib/semantic_space/semantic_space.h>
#include <idmlib/semantic_space/explicit_semantic_space.h>
#include <idmlib/semantic_space/document_vector_space.h>
#include <idmlib/semantic_space/semantic_space_builder.h>
#include <idmlib/semantic_space/explicit_semantic_interpreter.h>
#include <idmlib/semantic_space/term_doc_matrix_defs.h>
#include <idmlib/similarity/document_similarity_index.h>
#include <idmlib/util/collection_file_util.h>
#include <idmlib/util/FSUtil.hpp>

#define DOCSIM_TEST

using namespace idmlib::ssp;

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
		// Initialize

		// Explicit semantic interpreter initialized with wiki knowledge
		boost::shared_ptr<SemanticSpace> pWikiESSpace(new ExplicitSemanticSpace(esasspPath, SemanticSpace::LOAD));
#ifdef DOCSIM_TEST
		//pWikiESSpace->Print();
#endif
		pEsaInterpreter_.reset(new ExplicitSemanticInterpreter(pWikiESSpace));

		// create or load pre-processing data of collection (documents set)
		// can use indexed data..
		if (rebuild) {
		    idmlib::util::TimeChecker timer("Pre Process Collection(documents)");
			pDocVecSpace_.reset(new DocumentVectorSpace(colsspPath, SemanticSpace::CREATE));
			boost::shared_ptr<SemanticSpaceBuilder> pCollectionBuilder(
					new SemanticSpaceBuilder(pDocVecSpace_, laResPath, colBasePath, maxDoc));
			pCollectionBuilder->Build();
		}
		else {
			pDocVecSpace_.reset(new DocumentVectorSpace(colsspPath, SemanticSpace::LOAD));
		}
#ifdef DOCSIM_TEST
		pDocVecSpace_->Print();
#endif
		// similarity index
		pDocSimIndex_.reset(new DocumentSimilarityIndex(docSimPath, thresholdSim));
	}

	~DocumentSimilarity()
	{
	}

public:

	void DoSim();


private:
	boost::shared_ptr<SemanticInterpreter> pEsaInterpreter_;
	boost::shared_ptr<SemanticSpace> pDocVecSpace_; // processed collection data

	boost::shared_ptr<DocumentSimilarityIndex> pDocSimIndex_;


#if TO_DEL

	/**
	 * @brief Compute similarity for all pair of documents & build index
	 */

	bool Compute();

	/**
	 * @breif Build Interpretation Vectors for all documents in the collection
	 */
	bool buildInterpretationVectors();

	/**
	 * @breif Compute similarities between all pairs of documents.
	 */
	bool computeSimilarities();

	bool GetSimDocIdList(
			uint32_t docId,
			uint32_t maxNum,
			std::vector<std::pair<uint32_t, float> >& result);

private:
	bool getScdFileListInDir(const std::string& scdDir, std::vector<std::string>& fileList);

#endif

private:
	boost::shared_ptr<idmlib::util::CollectionFileUtil> colFileUtil_;

#if TO_DEL
	std::string colBasePath_;
	std::string scdPath_;
	izenelib::util::UString::EncodingType encoding_;
	boost::shared_ptr<DocumentVectorSpace> pDocVecSpace_;

	///
	boost::shared_ptr<SemanticInterpreter> pSSPInter_;
	boost::shared_ptr<IDManager > pIdManager_;

	typedef std::map<termid_t, count_t> termid_df_map;
	termid_df_map termid2DF_;

	typedef std::vector< std::vector<weight_t> > docIVecsT;
	docIVecsT docIVecs_;

	///:~ docid_index_map docid2Index_;
	///
#endif
};

NS_IDMLIB_SIM_END

#endif /* DOCUMENT_SIMILARITY_H_ */
