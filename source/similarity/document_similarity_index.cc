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
		bool ret = index_tc_->GetVector(conIndexId, conceptIndexVec);

		if (ret)
		{
            // score accumulation based on inverted index
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
		}

		// update inverted index
		conceptIndexVec.value.push_back(std::make_pair(docid, v));
		index_tc_->SetVector(conIndexId, conceptIndexVec);
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

void DocumentSimilarityIndex::accumulate_weight_tc_(
        std::map<docid_t, weight_t>& docWegtMap, docid_t& docid, docid_t& conceptId, weight_t& conW)
{
    docid_t candidateDocId;
    weight_t canDocW;

    doc_sp_vector conceptIndexVec;
    bool ret = index_tc_->GetVector(conceptId, conceptIndexVec);

    if (ret)
    {
        // score accumulation based on inverted index
        doc_sp_vector::ValueType::iterator conIter;
        for (conIter = conceptIndexVec.value.begin(); conIter != conceptIndexVec.value.end(); conIter ++)
        {
            candidateDocId = conIter->first;
            canDocW = conIter->second;
            // accumulate, fill weight table
            if (docWegtMap.find(candidateDocId) == docWegtMap.end()) {
                docWegtMap.insert(std::make_pair(candidateDocId, canDocW * conW));
            }
            else {
                docWegtMap[candidateDocId] += canDocW * conW;
            }
        }
    }

    // update inverted index
    conceptIndexVec.value.push_back(std::make_pair(docid, conW));
    index_tc_->SetVector(conceptId, conceptIndexVec);

}

void DocumentSimilarityIndex::accumulate_weight_(
        std::map<docid_t, weight_t>& docWegtMap, docid_t& docid, docid_t& conceptId, weight_t& conW)
{
    docid_t candidateDocId;
    weight_t canDocW;

    InterpretVector conceptIndexVec;
    bool ret = getConceptIndex(conceptId, conceptIndexVec);
    cout << " concept: " << conceptId << " ";
    if (ret)
    {
        // score accumulation based on inverted index
    	InterpretVector::iterator conIter;
        for (conIter = conceptIndexVec.begin(); conIter != conceptIndexVec.end(); conIter ++)
        {
            candidateDocId = conIter->first;
            canDocW = conIter->second;
            cout << "("<<candidateDocId<<","<<canDocW<<") ";
            // accumulate, fill weight table
            if (docWegtMap.find(candidateDocId) == docWegtMap.end()) {
                docWegtMap.insert(std::make_pair(candidateDocId, canDocW * conW));
            }
            else {
                docWegtMap[candidateDocId] += canDocW * conW;
            }
        }
    }
    cout << endl;

    // update inverted index
    conceptIndexVec.push_back(std::make_pair(docid, conW));
    updateConceptIndex(conceptId, conceptIndexVec);
}

void DocumentSimilarityIndex::basicInvertedIndexJoin_(docid_t& docid, InterpretVector& interVec)
{
    // cadidate docs with weights to given doc
    std::map<docid_t, weight_t> docWegtMap;

    docid_t conceptId;
    weight_t conW;

    InterpretVector::iterator vecIter;
    for (vecIter = interVec.begin(); vecIter != interVec.end(); vecIter ++)
    {
        conceptId = vecIter->first;
        conW = vecIter->second;

        accumulate_weight_(docWegtMap, docid, conceptId, conW);
//        if (storageType_ == DB_TOKYO_CABINET) {
//            accumulate_weight_tc_(docWegtMap, docid, conceptId, conW);
//        }
//        else if (storageType_ == DB_LevelDB) {
//
//        }
    }

    // Output doc-pairs, todo build doc similarity index..
    std::map<docid_t, weight_t>::iterator miter;
    for (miter = docWegtMap.begin(); miter != docWegtMap.end(); miter++)
    {
        if (miter->second > this->thresholdSim_) {
            // output
            cout << "(" << docid <<"," << miter->first <<"  " << miter->second<<")" << endl;
            outputSimDocPair(docid, miter->first, miter->second);
            outputSimDocPair(miter->first, docid, miter->second);
        }
    }
}
