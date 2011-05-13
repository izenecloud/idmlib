
#include <idmlib/idm_types.h>
#include <idmlib/similarity/document_similarity.h>
#include <idmlib/semantic_space/term_doc_matrix_defs.h>
//#include <idmlib/similarity/cosine_similarity.h>
#include <idmlib/similarity/simple_similarity.h>
#include <la/util/UStringUtil.h>
#include <ir/id_manager/IDManager.h>

using namespace idmlib::ssp;
using namespace idmlib::sim;


bool DocumentSimilarity::ComputeAll()
{
	return false;
}

bool DocumentSimilarity::Compute()
{
	// preprocess .. gather TF, DF
	if (!buildInterpretationVectors())
		return false;

	// compute similarity & build inverted index..
	if (!computeSimilarities())
		return false;

	return true;
}

bool DocumentSimilarity::buildInterpretationVectors()
{
	if (!pSSPInter_) {
		DLOG(WARNING) << "Semantic Interpreter has not been initialized." << endl;
		return false;
	}

	std::vector<std::string> scdFileList;
	if (!getScdFileListInDir(scdPath_, scdFileList)) {
		return false;
	}

	typedef std::vector<std::pair<izenelib::util::UString, izenelib::util::UString> >::iterator doc_properties_iterator;

    docid_t docid = 0;
    docid_t last_docid = 0;
    count_t doc_count = 0;

	for (size_t i = 0; i < scdFileList.size(); i++)
	{
		//std::cout << scdFileList[i] << std::endl;
		std::string scdFile = scdFileList[i];
		izenelib::util::ScdParser scdParser(encoding_);
		if(!scdParser.load(scdFile) )
		{
		  DLOG(WARNING) << "Load scd file failed: " << scdFile << std::endl;
		  return false;
		}

		// parse SCD
		izenelib::util::ScdParser::iterator iter = scdParser.begin();
		for (; iter != scdParser.end(); iter ++)
		{
			izenelib::util::SCDDocPtr pDoc = *iter;

			std::vector<weight_t> IVec; //

			doc_properties_iterator proIter;
			for (proIter = pDoc->begin(); proIter != pDoc->end(); proIter ++)
			{
				izenelib::util::UString propertyName = proIter->first;
				const izenelib::util::UString & propertyValue = proIter->second;
				propertyName.toLowerString();

				if ( propertyName == izenelib::util::UString("docid", encoding_) ) {
					last_docid = docid;
					bool ret = pIdManager_->getDocIdByDocName(propertyValue, docid, false);
					if (!ret) {
						DLOG(WARNING) << "Document (" << propertyValue << ") does not existed: !" << std::endl;
						return false;
					}
				}
				else if ( propertyName == izenelib::util::UString("title", encoding_) ) {
					//std::cout << la::to_utf8(proIter->second) << std::endl;
				}
				else if ( propertyName == izenelib::util::UString("content", encoding_)) {
					pSSPInter_->interpret(propertyValue, IVec);
				}
			}

			if (docid == last_docid)
				return false;

			docid2Index_.insert( make_pair(docid, doc_count ++) );
			docIVecs_.push_back( IVec ); // ...
		}

		// idf factor
		// pSSPInter_->getTermDF(termid)
		// for doc in docIVecs
		//
	}

	return true;
}

bool DocumentSimilarity::computeSimilarities()
{
	count_t N = docIVecs_.size();

	weight_t* psimMatrix = new weight_t[N*N];
	std::memset((void*)psimMatrix, 0, sizeof(weight_t)*(N*N));

	// upper triangular matrix
	weight_t* pw;
	for (size_t i = 0; i < N; i ++) {
		for (size_t j = i + 1; j < N; j ++) {
			pw = psimMatrix + (N * i) + j;
			//:*pw = CosineSimilarity::Sim(docIVecs_[i], docIVecs_[j]); // similarity
		}
	}

	/// build inverted index
	///

	// doc similarity
	for (size_t i = 0; i < N; i++)
	{
		for (size_t j = 0; j < N; j++)
		{
			cout << setw (10) << *(psimMatrix + (N * i) + j) ;
		}
		cout << endl;
	}
	return false;
}

bool DocumentSimilarity::GetSimDocIdList(
		uint32_t docId,
		uint32_t maxNum,
		std::vector<std::pair<uint32_t, float> >& result)
{
	return false;
}

bool DocumentSimilarity::getScdFileListInDir(const std::string& scdDir, std::vector<std::string>& fileList)
{
	if ( exists(scdDir) )
	{
		if ( !is_directory(scdDir) ) {
			//std::cout << "It's not a directory: " << scdDir << std::endl;
			return false;
		}

		directory_iterator iterEnd;
		for (directory_iterator iter(scdDir); iter != iterEnd; iter ++)
		{
			std::string file_name = iter->path().filename();
			//std::cout << file_name << endl;

			if (izenelib::util::ScdParser::checkSCDFormat(file_name) )
			{
				izenelib::util::SCD_TYPE scd_type = izenelib::util::ScdParser::checkSCDType(file_name);
				if( scd_type == izenelib::util::INSERT_SCD ||scd_type == izenelib::util::UPDATE_SCD )
				{
					fileList.push_back( iter->path().string() );
				}
			}
		}

		if (fileList.size() > 0) {
			return true;
		}
		else {
			//std::cout << "There is no scd file in: " << scdDir << std::endl;
			return false;
		}
	}
	else
	{
		//std::cout << "File path dose not existed: " << scdDir << std::endl;
		return false;
	}
}