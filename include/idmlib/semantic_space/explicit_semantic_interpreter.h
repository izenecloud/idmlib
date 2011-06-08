/**
 * @file idmlib/semantic_space/semantic_interpreter.h
 * @author Zhongxia Li
 * @date Mar 10, 2011
 * @brief Explicit Semantic Analysis, refer to
 * Evgeniy Gabrilovich and Shaul Markovitch. (2007).
 * "Computing Semantic Relatedness using Wikipedia-based Explicit Semantic Analysis,"
 * Proceedings of The 20th International Joint Conference on Artificial Intelligence
 * (IJCAI), Hyderabad, India, January 2007.
 */
#ifndef IDMLIB_SSP_SEMANTIC_INTERPRETER_H_
#define IDMLIB_SSP_SEMANTIC_INTERPRETER_H_

#include <iostream>

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include <util/ustring/UString.h>
#include <ir/index_manager/index/IndexReader.h>
#include <ir/index_manager/index/Indexer.h>
#include <ir/index_manager/index/LAInput.h>
#include <ir/index_manager/index/Indexer.h>
#include <ir/index_manager/index/Term.h>
#include <ir/index_manager/index/AbsTermReader.h>
#include <ir/id_manager/IDManager.h>
#include <idmlib/idm_types.h>
#include <idmlib/util/idm_analyzer.h>
#include <idmlib/semantic_space/izene_index_helper.h>
#include <idmlib/semantic_space/term_doc_matrix_defs.h>

NS_IDMLIB_SSP_BEGIN

class SemanticInterpreter
{
public:
    /**
     * Interpreter for Explicit Semantic Analysis
     * @param pSSpaceWiki inverted index of Wiki concepts resource
     *
     * @deprecated The SemanticSpace with db storage it's quite slow,
     * so it is replaced by a solution which using Indexer of SF1.
     */
	SemanticInterpreter(boost::shared_ptr<SemanticSpace>& pSSpaceWiki)
	: sspWikiIndex_(pSSpaceWiki)
	{
	    conceptNum_ = sspWikiIndex_->getDocNum();
	}

	/**
	 * Interpreter for Explicit Semantic Analysis
	 * @param pIndexer inverted index of Wiki concepts resource
	 */
	SemanticInterpreter(boost::shared_ptr<Indexer>& pWikiIndexer, const string& laResPath, const string& colBasePath)
	:pWikiIndexer_(pWikiIndexer)
	{
	    // Indexer
	    BOOST_ASSERT(pWikiIndexer_);
	    conceptNum_ = pWikiIndexer_->getIndexReader()->numDocs();
	    cout << "[SemanticInterpreter] init, wiki concepts: " << conceptNum_ << endl;

	    // LA, remove stopwords performs better
	    pIdmAnalyzer_.reset(
	            new idmlib::util::IDMAnalyzer(laResPath, la::ChineseAnalyzer::maximum_match, true)
	            );
	    BOOST_ASSERT(pIdmAnalyzer_);

	    // ID Manager
	    string idDir = colBasePath + "/collection-data/default-collection-dir/id";
	    pIdManager_.reset( new izenelib::ir::idmanager::IDManager(idDir) );
	    BOOST_ASSERT(pIdManager_);
	}

	 ~SemanticInterpreter()
	{
	}

public:

	/**
	 * Interpret semantic of a text (document) to interpretation vector
	 * @brief maps fragments of natural language text (document) into a interpretation vector
	 * @param[IN] text a input natural language text or a document
	 * @param[OUT] interVector interpretation vector of the input text (document)
	 * @return true on success, false on failure.
	 */
	InterpretVector& interpret(docid_t& docid, const UString& text)
	{
	    // analyze text
	    {
            laInput_.resize(0);
            pIdmAnalyzer_->GetTermIdList(pIdManager_.get(), text, laInput_);
	    }

	    representText_();

	    interpretText_();

		return interpretVec_;
	}

	void sortInterpretVector()
	{
	    std::sort(interpretVec_.begin(), interpretVec_.end(), sort_second());
	}

	void printInterpretVector()
	{
	    for (size_t i = 0; i < interpretVec_.size(); i ++)
	    {
	        cout << "("<<interpretVec_[i].first << "," << interpretVec_[i].second << ") ";
	    }
	    cout << endl;
	}

private:
	void representText_()
	{
	    // init
	    representVec_.resize(0);

	    // unique terms
	    std::set<termid_t> unique_term_set;
	    std::pair<set<termid_t>::iterator,bool> unique_term_ret;

	    termid_t termid;
	    term_doc_tf_map termid2doctf;
	    for (TermIdList::iterator iter = laInput_.begin(); iter != laInput_.end(); iter ++)
	    {
	        // term id
	        termid = iter->termid_;

	        // DF, TF in document
	        unique_term_ret = unique_term_set.insert(termid);
	        if (unique_term_ret.second == true) {
//              if (isPreLoadTermInfo_) {
                    // update DF
                    if (termid2df_.find(termid) != termid2df_.end()) {
                        termid2df_[termid] ++;
                    }
                    else {
                        termid2df_.insert(term_df_map::value_type(termid, 1));
                    }
//	            }
	            // update doc TF
	            termid2doctf.insert(term_doc_tf_map::value_type(termid, 1));
	        }
	        else {
	            // update doc TF
	            termid2doctf[termid] ++;
	        }
	    }

	    //term_sp_vector representDocVec;
	    weight_t wegt=0; // df, idf, tf*idf
	    weight_t tf;
	    count_t doc_length = laInput_.size();
	    ///cout << "text len: " << doc_length << endl;
	    for (term_doc_tf_map::iterator dtfIter = termid2doctf.begin(); dtfIter != termid2doctf.end(); dtfIter++)
	    {
	        tf = dtfIter->second / doc_length; // normalized tf
            /*
	        if (isPreLoadTermInfo_) {
	            wegt = pTermInfoReader_->getDFByTermId(dtfIter->first); // df
	            if (wegt < 1) { // non-zero
	                continue;
	            }
	            wegt = std::log( pTermInfoReader_->getDocNum() / wegt); // idf
	            wegt *= tf; // weight = tf * idf
	            if (wegt < thresholdWegt_) {
	                continue;
	            }
	        } */
	        //representDocVec.value.push_back(std::make_pair(term_index, wegt));
	        //pDocRepVectors_->SetVector(doc_index, representDocVec);

	        wegt = tf;
	        representVec_.push_back(std::make_pair(dtfIter->first, wegt)); // (termid, weight)
	    }
	}

	void interpretText_()
	{
	    // The weights of interpretation vector (of which each element is the weight of the text to a concept),
	    // are calculated by incrementally weight accumulation while traverse entries for text terms
	    // over the inverted index of wiki concepts, conceptWeightMap is used to accumulate weights.

        std::map<docid_t, weight_t> conceptWeightMap;

        for (size_t i = 0; i < representVec_.size(); i++)
        {
            ///cout << "("<<representVec_[i].first << ", "<< representVec_[i].second<<") ==> ";
            weight_t wtext = representVec_[i].second;

            // entry,  todo, mining property
            Term term("Content", representVec_[i].first);

            izenelib::ir::indexmanager::IndexReader* indexReader = pWikiIndexer_->getIndexReader();
            izenelib::ir::indexmanager::TermReader*
            pTermReader = indexReader->getTermReader(idmlib::ssp::IzeneIndexHelper::COLLECTION_ID);
            if (!pTermReader) {
                continue;
            }

            bool find = pTermReader->seek(&term);
            if (find)
            {
                // term indexed
                weight_t DF = pTermReader->docFreq(&term);
                weight_t IDF = std::log(conceptNum_ / DF);
                ///cout <<"DF: "<< DF << ", IDF: " << IDF <<" ";

                izenelib::ir::indexmanager::TermDocFreqs* pTermDocReader = NULL;
                pTermDocReader = pTermReader->termDocFreqs();

                weight_t TF = 0;
                if (pTermDocReader != NULL) {
                    // iterate concepts indexed by term
                    while (pTermDocReader->next()) {
                        docid_t conceptId = pTermDocReader->doc();
                        TF = pTermDocReader->freq();

                        size_t docLen =
                        indexReader->docLength(conceptId, idmlib::ssp::IzeneIndexHelper::getPropertyIdByName("Content"));
                        ///cout << "( doc:"<<conceptId<<", tf:"<<TF<<", doclen:"<<docLen<<", tf:"<<(TF / docLen) <<" ) " ;

                        // TF*IDF
                        weight_t wcon = IDF * (TF / docLen);
                        if (conceptWeightMap.find(conceptId) != conceptWeightMap.end())
                        {
                            // xxx, wtext is TF of text, IDF should be statistic in corpus where text is from ?
                            conceptWeightMap[conceptId] += (wtext*IDF) * wcon;
                        }
                        else
                        {
                            conceptWeightMap.insert(std::make_pair(conceptId, (wtext*IDF) * wcon));
                        }
                    }
                    ///cout << endl;
                }

                if (pTermDocReader) {
                    delete pTermDocReader;
                    pTermDocReader = NULL;
                }
            }

            if (pTermReader) {
                delete pTermReader;
                pTermReader = NULL;
            }
        }

        // construct interpretation vector
        weight_t w;
        std::map<docid_t, weight_t>::iterator cwIter;
        weight_t vecLength = 0; // (or magnitude) for normalization
        for (cwIter = conceptWeightMap.begin(); cwIter != conceptWeightMap.end(); cwIter++)
        {
            vecLength +=  cwIter->second * cwIter->second;
        }
        vecLength = std::sqrt(vecLength);

        interpretVec_.resize(0); // init
        for (cwIter = conceptWeightMap.begin(); cwIter != conceptWeightMap.end(); cwIter++)
        {
             w = cwIter->second / vecLength;
             if (w > thresholdWegt_)
                 interpretVec_.push_back(make_pair(cwIter->first, w));
        }
	}

public:
	/**
	 * @brief Convert a document from representation vector to interpretation vector of ESA
	 * Given repreDocVec <wi> with weights <vi>, for wi weight to concept cj is kj,
	 * Then interDocVec is <dcj>, where dcj = SUM vi*kj for all wi in <wi>,
	 * dcj is the weight of DOC to CONCEPT cj.
	 * @param[IN] repreDocVec
	 * @param[OUT] interDocVec
	 *
	 * @deprecated the idea is the same with another interface
	 */
	bool interpret(term_sp_vector& repreDocVec, interpretation_vector_type& interDocVec)
	{
		/* Here for performance, did not calculate the weight of a doc to a concept at a time,
		   but incrementally accumulate weights to each concepts by traverse entries of words(terms)
		   of repreDocVec on inverted index of wiki concepts (word-concept), also used a concept-weight
		   map to speed up accumulating. */
		std::map<docid_t, weight_t> conceptWeightMap;

		termid_t termid;
		weight_t wdoc;
		term_sp_vector::ValueType::iterator tIter;
		docid_t conceptId;
		weight_t wcon;
		doc_sp_vector::ValueType::iterator cIter;
		// for all representation words(terms) of the document
		for (tIter = repreDocVec.value.begin(); tIter != repreDocVec.value.end(); tIter ++)
		{
			termid = tIter->first;
			wdoc = tIter->second;

			// for entry of current word(term), calculate sub weights to concepts
			doc_sp_vector consVec;
			sspWikiIndex_->getVectorByTermid(termid, consVec);
			for (cIter = consVec.value.begin(); cIter != consVec.value.end(); cIter ++)
			{
				conceptId = cIter->first;
				wcon = cIter->second;

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
             if (w > thresholdWegt_)
                interDocVec.value.push_back(make_pair(cwIter->first, w));
        }

		return true;
	}

private:
	static const weight_t thresholdWegt_ = 0.02;

private:
	boost::shared_ptr<SemanticSpace> sspWikiIndex_;
	count_t conceptNum_;

	// wiki indexer
	boost::shared_ptr<Indexer> pWikiIndexer_;
	// LA
	boost::shared_ptr<idmlib::util::IDMAnalyzer> pIdmAnalyzer_;
	TermIdList laInput_;
	// IDM
	boost::shared_ptr<izenelib::ir::idmanager::IDManager> pIdManager_;

	typedef std::map<termid_t, weight_t> term_doc_tf_map;

	term_df_map termid2df_;
	std::vector<std::pair<termid_t, weight_t> > representVec_; // (term, weight)
	InterpretVector interpretVec_; // (concept(doc), weight)
};

typedef SemanticInterpreter ExplicitSemanticInterpreter;

NS_IDMLIB_SSP_END

#endif /* SEMANTIC_INTERPRETER_H_ */
