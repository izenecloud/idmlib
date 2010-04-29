/*
 * t_dc.cc
 *
 *  Created on: Mar 24, 2010
 *      Author: eric
 */

#include <ml/ClassificationManager.h>
#include <idmlib/dc/DocumentUtil.h>
#include "ScdDocumentLoader.h"
#include <sf1v5/configuration-manager/CollectionPath.h>
#include <sf1v5/configuration-manager/CollectionMeta.h>
#include <sf1v5/configuration-manager/PropertyConfig.h>
#include <sf1v5/la-manager/AnalysisInformation.h>

using namespace sf1v5;

void test1()
{
	// construct collection path
	CollectionPath path;
	path.setBasePath("/home/eric/test/corpus/dc");
	path.setScdPath("/home/eric/test/corpus/dc/scd");
	path.setIndexPath("/home/eric/test/corpus/dc/index-data");
	path.setMiningPath("/home/eric/test/corpus/dc/mining-data");
	path.setLogPath("/home/eric/test/corpus/dc/log-data");

	// construct collection meta
	CollectionMeta meta;
	meta.setName("dc");
	meta.setEncoding(wiselib::UString::UTF_8);
	meta.setRankingMethod("ranker1");
	meta.setDateFormat("none_time_t");
	meta.setCollectionPath(path);

	// construct property config
	PropertyConfig config1;
	config1.setName("DOCID");
	config1.setType(STRING_PROPERTY_TYPE);
	config1.setMaxLength(1000);
	config1.setIsIndex(true);
	config1.setIsMining(false);

	PropertyConfig config2;
	config2.setName("TITLE");
	config2.setType(STRING_PROPERTY_TYPE);
	config2.setMaxLength(1000);
	config2.setIsIndex(true);
	config2.setIsMining(true);

	PropertyConfig config3;
	config3.setName("CONTENT");
	config3.setType(STRING_PROPERTY_TYPE);
	config3.setMaxLength(1000);
	config3.setIsIndex(true);
	config3.setIsMining(true);

	PropertyConfig config4;
	config4.setName("CATEGORY");
	config4.setType(STRING_PROPERTY_TYPE);
	config4.setMaxLength(1000);
	config4.setIsIndex(false);
	config4.setIsMining(false);

	AnalysisInfo analysisInfo;
//	analysisInfo.tokenizerNameList_.insert("tok_");
	analysisInfo.analyzerId_    = "la_ngram";
	config1.setAnalysisInfo(analysisInfo);
	config2.setAnalysisInfo(analysisInfo);
	config3.setAnalysisInfo(analysisInfo);
	config4.setAnalysisInfo(analysisInfo);

	meta.insertPropertyConfig(config1);
	meta.insertPropertyConfig(config2);
	meta.insertPropertyConfig(config3);
	meta.insertPropertyConfig(config4);

	// start loading
	std::vector<ScdDocument> docs;
	ScdDocumentLoader loader(meta);
	loader.loadFromDir(docs, "/home/eric/test/corpus/dc/scd");
	ml::ClassificationManager<ScdDocument> dcMgr("/home/eric/test/corpus/dc/dc_model", LR);
	dcMgr.train(docs);

//	dcMgr.loadSchema();
//	dcMgr.loadModels();
//
//	for (std::vector<ScdDocument>::iterator it = docs.begin();
//			it != docs.end(); ++it)
//	{
//		dcMgr.predict(*it);
//	}
//
//	// destroy memory models
//	dcMgr.destroyModels();
}

void test2()
{
	std::string path = "/home/eric/test/corpus/dc/dc_model";
	ml::ClassifierType type = LR;
	ml::ClassificationManager<ScdDocument> dcMgr(path, type);
	dcMgr.loadSchema();
	dcMgr.loadModels();

//	DocumentVector docVector(path);
//	docVector.indexWorker(termIndex_->getIndexReader(), propertyNames_);
//	docVector.begin();
//	int size = 1000;
//	std::vector<ScdDocument> docBuffer;
//	docVector.getNextData(docBuffer, size);
//	while (docBuffer.size() > 0)
//	{
//		size_t nDoc = docBuffer.size();
//		//
//		for (size_t i=0; i<nDoc; ++i)
//		{
//			dcMgr.predict(docBuffer[i]);
//			// TODO: save predict result
//		}
//
//		docBuffer.clear();
//		docVector.getNextData(docBuffer, size);
//	}
//	docVector.end();
//
//
//	// predict
//	for (std::vector<ScdDocument>::const_iterator it = docs.begin();
//			it != docs.end(); ++it)
//	{
//		dcMgr.predict(*it);
//	}
//
//	// destroy memory models
//	dcMgr.destroyModels();
}


int main()
{
	test1();
}
