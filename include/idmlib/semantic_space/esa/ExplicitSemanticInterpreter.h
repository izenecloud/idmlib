/**
 * @file ExplicitSemanticAnalyzer.h
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

#include <idmlib/idm_types.h>

#include "WikiIndex.h"
#include "MemWikiIndex.h"
#include "SparseVectorSetFile.h"


NS_IDMLIB_SSP_BEGIN

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
		pWikiIndex_.reset(new MemWikiIndex(wikiIndexPath));
		cout << "loading wiki index.."<<endl;
		pWikiIndex_->load();
	}

public:
	bool interpret()
	{
		DLOG(INFO) << "start interpreting" <<endl;

		SparseVectorSetIFileType inf(docSetPath_+"/doc_rep.tmp");
		if (!inf.open())
		{
		    return false;
		}

		SparseVectorSetOFileType interf(docSetPath_+"/doc_int.vec");
		interf.open();

		size_t total = 0;
		while(inf.next())
		{
			SparseVectorType& docvec = inf.get();
#ifdef DOC_SIM_TEST
			docvec.print();
#endif
			SparseVectorType ivec(docvec.rowid);
			interpret_(docvec, ivec);

#ifdef DOC_SIM_TEST
			cout << "**interpreted : " << endl;
			ivec.print();
			cout << endl;
#endif
			interf.put(ivec);

			++total;

			if ((total % 1000) == 0)
				LOG(INFO) << "interpreted: " <<total << endl;
		}

		inf.close();
		interf.close();

		DLOG(INFO) << "end interpreting" <<endl;

		return true;
	}

private:

	void interpret_(SparseVector<>& docvec, SparseVector<>& ivec)
	{
	    // The weights of interpretation vector (of which each element is the weight of the doc to a concept),
	    // are calculated by incrementally weight accumulation while traverse entries for text terms
	    // over the inverted index of wiki concepts, conceptWeightMap is used to accumulate weights.

		std::map<docid_t, weight_t> conceptWeightMap;

		uint32_t termid, conceptId;
		weight_t wdoc, wcon;
		SparseVector<>::list_iter_t tIter, cIter;

		// for all terms in representation of the document
		for (tIter = docvec.list.begin(); tIter != docvec.list.end(); tIter ++)
		{
			termid = tIter->itemid;
			wdoc = tIter->value;

			// for entry of current word(term), calculate sub weights to concepts
			SparseVectorType& consVec = pWikiIndex_->getInvertedList(termid);
			for (cIter = consVec.list.begin(); cIter != consVec.list.end(); cIter ++)
			{
				conceptId = cIter->itemid;
				wcon = cIter->value;

				if (conceptWeightMap.find(conceptId) != conceptWeightMap.end())
				{
					conceptWeightMap[conceptId] += wdoc * wcon;
				}
				else {
					conceptWeightMap.insert(std::make_pair(conceptId, wdoc * wcon));
				}
			}
		}

		std::map<docid_t, weight_t>::iterator cwIter;
#if 1 // normalization (convert to unit-length vector)
		// vector length, or magnitude
		weight_t vecLength = 0;
		for (cwIter = conceptWeightMap.begin(); cwIter != conceptWeightMap.end(); cwIter++)
		{
		    vecLength +=  cwIter->second * cwIter->second;
		}
		vecLength = std::sqrt(vecLength);
#else // not normalize
		weight_t vecLength = 1;
#endif
		// set doc interpretation vector
		weight_t w;
        for (cwIter = conceptWeightMap.begin(); cwIter != conceptWeightMap.end(); cwIter++)
        {
             w = cwIter->second / vecLength;
             //if (w > thresholdWegt_)
             //   interDocVec.value.push_back(make_pair(cwIter->first, w));
                ivec.insertItem(cwIter->first, w);
        }
	}

private:
	std::string wikiIndexPath_;
	std::string docSetPath_;

	boost::shared_ptr<MemWikiIndex> pWikiIndex_;
};

NS_IDMLIB_SSP_END

#endif /* EXPLICIT_SEMANTIC_INTERPRETER_H_ */
