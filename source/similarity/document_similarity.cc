
#include <idmlib/idm_types.h>
#include <idmlib/similarity/document_similarity.h>
#include <idmlib/semantic_space/term_doc_matrix_defs.h>
#include <idmlib/similarity/simple_similarity.h>
#include <la/util/UStringUtil.h>
#include <ir/id_manager/IDManager.h>
#include <ir/index_manager/index/IndexReader.h>
#include <ir/index_manager/index/Indexer.h>

using namespace idmlib::ssp;
using namespace idmlib::sim;
using namespace idmlib::util;
using namespace izenelib::ir::indexmanager;

void DocumentSimilarity::DoSim()
{
#ifdef IDM_SSP_TIME_CHECKER
    idmlib::util::TimeChecker DocSimtimer("Do document similarity");
#endif



	// Compute similarities between all pair of documents

	std::vector<docid_t>& docList = pDocVecSpace_->getDocList();
	size_t docNum = docList.size();
	size_t progress = 0;

	DLOG(INFO) << "Start document similarity.  doc number: " << docNum << endl;

	std::vector<docid_t>::iterator docIter;
	docid_t docid;
	for (docIter = docList.begin(); docIter != docList.end(); docIter ++) {
		docid = *docIter;

		term_sp_vector representDocVec;
		pDocVecSpace_->getVectorByDocid(docid, representDocVec);

#ifdef SSP_TIME_CHECKER
		idmlib::util::TimeChecker timer("interpret");
#endif
		// interpret a document
		//idmlib::util::TimeChecker timer("interpret a document");
		interpretation_vector_type interpretationDocVec;
		pEsaInterpreter_->interpret(representDocVec, interpretationDocVec);
		//timer.EndPoint();
#ifdef SSP_TIME_CHECKER
		timer.EndPoint();
		timer.Print();
#endif

//		stringstream ss;
//		ss << "interpretation vector(" << *docIter << ")";
//		idmlib::ssp::PrintSparseVec(interpretationDocVec, ss.str());
#if 0
//*	    test sort by concepts weight
		std::vector<pair<docid_t, weight_t> > con_weight_list;
		interpretation_vector_type::ValueType::iterator ivIter;
		for (ivIter = interpretationDocVec.value.begin(); ivIter != interpretationDocVec.value.end(); ivIter++)
		{
			con_weight_list.push_back(std::make_pair(ivIter->first, ivIter->second));
		}
		std::sort(con_weight_list.begin(), con_weight_list.end(), sort_second());
		std::vector<pair<docid_t, weight_t> >::iterator cwIter;
		int i = 0;
		cout << "[ Doc: " << docid <<" (concept-weight)] : " ;
		for (cwIter = con_weight_list.begin(); cwIter != con_weight_list.end(); cwIter ++)
		{
			cout << "(" << cwIter->first <<", " << cwIter->second << ") ";
			i++;
			if (i >= 1000)
				break;
		}
		cout << "concepts: "<< i << endl;
//*/
#endif
		// Build document similarity index ..
//		pDocSimIndex_->InertDocument(docid, interpretationDocVec);

		if ((++progress) % 100 == 0 || progress >= docNum) {
		    DLOG(INFO) << (progress * 100.0f / docNum)  << "% - total " << docNum << endl;
		}
	}

//	pDocSimIndex_->FinishInert();

	DLOG(INFO) << "End document similarity." << endl;
}

bool DocumentSimilarity::computeSimilarity()
{
    /*
    docid_t docid = 1;
    izenelib::util::UString udoc(
            "打浦路隧道是上海第一条跨越黄浦江、连接浦东与浦西的隧道，为单管两车道相向行驶。隧道浦西出口在打浦路，浦东出口在周家渡路，全长2,736米。"
"1960年，上海市隧道工程局成立，经过6年前期准备工作，1965年确定在打浦路与周家渡间建设越江隧道，同年6月动工，1970年9月建成。",
            izenelib::util::UString::UTF_8);

    InterpretVector& interVec = pEsaInterpreter_->interpret(docid, udoc);
    pEsaInterpreter_->sortInterpretVector();
    pEsaInterpreter_->printInterpretVector();

    return true;
    //*/

    std::vector<std::string> scdFileList;
    if (!SemanticSpaceBuilder::getScdFileList(colFileUtil_->getScdDir(), scdFileList)) {
        return false;
    }

    DLOG(INFO) << "Start document similarity." << endl;

    // parsing all SCD files
    for (size_t i = 1; i <= scdFileList.size(); i++)
    {
        std::string scdFile = scdFileList[i-1];
        izenelib::util::ScdParser scdParser(encoding_);
        if(!scdParser.load(scdFile) )
        {
          DLOG(WARNING) << "Load scd file failed: " << scdFile << std::endl;
          return false;
        }

        /// test
        std::vector<izenelib::util::UString> list;
        scdParser.getDocIdList(list);
        size_t totalDocNum = list.size();

        DLOG(INFO) << "SCD (" << i <<" / " << scdFileList.size() <<") total documents: " << totalDocNum << endl;

        // parse SCD
        size_t curDocNum = 0;
        for (izenelib::util::ScdParser::iterator iter = scdParser.begin(); iter != scdParser.end(); iter ++)
        {
            SCDDocPtr pDoc = *iter;
            processDocument_(pDoc);

            curDocNum ++;
            if (curDocNum % 1000 == 0) {
                DLOG(INFO) << "["<<scdFile<<"] processing: " << curDocNum<<" / total: "<<totalDocNum
                           << " - "<< curDocNum*100.0f / totalDocNum << "%" << endl;
            }

            ///test
            if (curDocNum > maxDoc_)
                break;
        }
    }

    //pDocSimIndex_->FinishInert();

    DLOG(INFO) << "End document similarity." << endl;
    return true;
}

void DocumentSimilarity::processDocument_(SCDDocPtr& pDoc)
{
    docid_t docId = 0;

    doc_properties_iterator proIter;
    for (proIter = pDoc->begin(); proIter != pDoc->end(); proIter ++)
    {
        izenelib::util::UString propertyName = proIter->first;
        const izenelib::util::UString & propertyValue = proIter->second;
        propertyName.toLowerString();

        if ( propertyName == izenelib::util::UString("docid", encoding_) ) {
            bool ret = pIdManager_->getDocIdByDocName(propertyValue, docId, false);
            if (ret) ;

        }
        if ( propertyName == izenelib::util::UString("title", encoding_) ) {

        }
        else if ( propertyName == izenelib::util::UString("content", encoding_)) {
            InterpretVector& interVec = pEsaInterpreter_->interpret(docId, propertyValue);
            //pEsaInterpreter_->sortInterpretVector(); //
            //pEsaInterpreter_->printInterpretVector(); //

            //pDocSimIndex_->InertDocument(docId, interVec); // <DOCID> property has to come first
        }
        else {
            continue;
        }
    }
}
