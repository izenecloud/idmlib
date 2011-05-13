#include <idmlib/similarity/document_similarity_index.h>

using namespace idmlib::sim;


void DocumentSimilarityIndex::basicInvertedIndexJoin_(
		docid_t& docid,
		interpretation_vector_type& interDocVec)
{
	// empty map from doc id to weight, forward
	std::map<docid_t, weight_t> docWegtMap;

	docid_t conIndexId;
	docid_t docid_candidate;
	weight_t v, k;

	interpretation_vector_type::ValueType::iterator docIter;
	for (docIter = interDocVec.value.begin(); docIter != interDocVec.value.end(); docIter++)
	{
		conIndexId = docIter->first;
		v = docIter->second;

		doc_sp_vector conceptIndexVec;
		bool ret =
		conceptDocIndex_->GetVector(conIndexId, conceptIndexVec);

		// Find match
		doc_sp_vector::ValueType::iterator conIter;
		for (conIter = conceptIndexVec.value.begin(); conIter != conceptIndexVec.value.end(); conIter ++)
		{
			docid_candidate = conIter->first;
			k = conIter->second;
			// accumulate, fill weight table
			if (docWegtMap.find(docid_candidate) == docWegtMap.end()) {
				docWegtMap.insert(std::make_pair(docid_candidate, v*k));
			}
			else {
				docWegtMap[docid_candidate] += v*k;
			}
		}

		// update inverted index
		conceptIndexVec.value.push_back(std::make_pair(docid, v));
		conceptDocIndex_->SetVector(conIndexId, conceptIndexVec);
	}

	// Output doc-pairs, todo build doc similarity index..
	std::map<docid_t, weight_t>::iterator miter;
	for (miter = docWegtMap.begin(); miter != docWegtMap.end(); miter++)
	{
		if (miter->second > this->thresholdSim_) {
			// output
			//cout << "(" << docid <<"," << miter->first <<", " << miter->second<<")" << endl;
			outputSimDocPair(docid, miter->first, miter->second);
			outputSimDocPair(miter->first, docid, miter->second);
		}
	}
}
