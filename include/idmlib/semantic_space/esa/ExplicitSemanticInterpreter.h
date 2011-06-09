/**
 * @file ExplicitSemanticInterpreter.h
 * @author Zhongxia Li
 * @date Jun 1, 2011
 * @brief Explicit Semantic Analysis
 *
 * Evgeniy Gabrilovich and Shaul Markovitch. (2007).
 * "Computing Semantic Relatedness using Wikipedia-based Explicit Semantic Analysis,"
 * Proceedings of The 20th International Joint Conference on Artificial Intelligence
 * (IJCAI), Hyderabad, India, January 2007.
 */
#ifndef EXPLICIT_SEMANTIC_INTERPRETER_H_
#define EXPLICIT_SEMANTIC_INTERPRETER_H_

//#define IZENE_INDEXER_ // not used

#include <idmlib/idm_types.h>

#include "WikiIndex.h"
#include "MemWikiIndex.h"
#include "SparseVectorSetFile.h"

#ifdef IZENE_INDEXER_
#include <idmlib/semantic_space/esa/izene_index_helper.h>
#include <ir/index_manager/index/Indexer.h>
#include <ir/index_manager/index/Term.h>
#include <ir/index_manager/index/AbsTermReader.h>
#include <ir/index_manager/index/IndexReader.h>
#endif

NS_IDMLIB_SSP_BEGIN

static double sTotalTime = 0;

class ExplicitSemanticInterpreter
{
	typedef float weight_t;

public:
	ExplicitSemanticInterpreter(
			const std::string& wikiIndexPath,
			const std::string& docSetPath )
	: wikiIndexPath_(wikiIndexPath)
	, docSetPath_(docSetPath)
	{
	    thresholdWegt_ = 0; // xxx

#ifndef IZENE_INDEXER_
		pWikiIndex_.reset(new MemWikiIndex(wikiIndexPath));
		cout << "loading wiki index.."<<endl;
		pWikiIndex_->load();
#else
		pIndexer_ = idmlib::ssp::IzeneIndexHelper::createIndexer(wikiIndexPath+"/izene_index");
		wikiDocNum_ = pIndexer_->getIndexReader()->numDocs();
#endif
	}

public:
	bool interpret(size_t maxCount = 0);


private:
	void interpret_(SparseVectorType& docvec, SparseVectorType& ivec);

#ifdef IZENE_INDEXER_
    void interpret_ir_(SparseVectorType& docvec, SparseVectorType& ivec);
#endif


private:
	std::string wikiIndexPath_;
	std::string docSetPath_;

	boost::shared_ptr<MemWikiIndex> pWikiIndex_;

	weight_t thresholdWegt_;

#ifdef IZENE_INDEXER_
	boost::shared_ptr<Indexer> pIndexer_;
	uint32_t wikiDocNum_;
#endif
};

NS_IDMLIB_SSP_END

#endif /* EXPLICIT_SEMANTIC_INTERPRETER_H_ */
