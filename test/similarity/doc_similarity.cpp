/**
 * @file doc_sim.cpp
 * @author Zhongxia Li
 * @date Mar 11, 2011
 * @brief Document Similarity Test
 */
#include <iostream>
#include <ctime>
using namespace std;

#include <boost/shared_ptr.hpp>
#include <boost/program_options.hpp>

#include <la/LA.h>

#include <idmlib/semantic_space/esa/DocumentRepresentor.h>
#include <idmlib/semantic_space/esa/ExplicitSemanticInterpreter.h>
#include <idmlib/similarity/all-pairs-similarity-search/data_set_iterator.h>
#include <idmlib/similarity/all-pairs-similarity-search/all_pairs_search.h>
#include <idmlib/similarity/all-pairs-similarity-search/all_pairs_output.h>

namespace po = boost::program_options;
using namespace idmlib::ssp;
using namespace idmlib::sim;

int main(int argc, char** argv)
{
	string wikiIndexdir; // resource data(Wiki) for explicit semantic analysis
	string laResPath;  // LA (CMA) resource path
	string colBasePath; // document collection to perform doc-similarity mining
	string docRepPath; // temporary file to store document represention vectors which possibly out of memory capacity
	string docSimPath; // document similarity index path
	float thresholdSim = 0.0001; // similarity threshold value
	uint32_t maxDoc = 0; // max number of documents to be processed, not limited if 0
	//bool rebuild = false;

	po::options_description desc("Allowed options");
	desc.add_options()
		("help,H", "produce help message")
		("wiki-index,W", po::value<std::string>(&wikiIndexdir), "resource data (Wiki) directory for explicit semantic analysis.")
		("la-resource,L", po::value<std::string>(&laResPath), "LA(CMA) resource path.")
		("doc-set,D", po::value<std::string>(&colBasePath), "collection to be processed.")
		("doc-represent-file,R", po::value<std::string>(&docRepPath), "document representation vectors file.")
		("doc-sim-index,I", po::value<std::string>(&docSimPath), "document similarity index path.")
		("threshold-sim,T", po::value<float>(&thresholdSim), "similarity threshold value.")
		("max-doc,M", po::value<uint32_t>(&maxDoc), "max doc count that will be processed.")
		//("rebuild-ssp-data,R", po::value<std::string>(), "whether rebuild collection s space data.")
	;
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if (vm.count("help")) {
		std::cout << desc << std::endl;
		return 0;
	}
	if (vm.empty()) {
		std::cout << desc << std::endl<< std::endl;
	}

	if (wikiIndexdir.empty()) {
	    wikiIndexdir = "./wiki/index";
	}
	cout << "Wikipedia index: " <<  wikiIndexdir << endl;

	if (laResPath.empty()) {
		laResPath = "/home/zhongxia/codebase/icma/db/icwb/utf8";
	}
	cout << "la-res-path: " <<  laResPath << endl;

	if (colBasePath.empty()) {
		colBasePath = "/home/zhongxia/codebase/sf1-revolution-dev/bin/collection/chinese-wiki-test";
	}
	cout << "document set: " <<  colBasePath << endl;

	if (docRepPath.empty()) {
	    docRepPath = "./docrep.tmp";
	}
	cout << "document representation temporary file: " <<  docRepPath << endl;

	if (docSimPath.empty()) {
		docSimPath = "./doc_sim";
	}
	cout << "document similarity index: " <<  docSimPath << endl;

	std::cout << "threshold-sim: " << thresholdSim << endl;
	std::cout << "max-doc: " << maxDoc << endl;

	/* if (vm.count("rebuild-ssp-data")) {
	    if (vm["rebuild-ssp-data"].as<std::string>() == "true" || vm["rebuild-ssp-data"].as<std::string>() == "t") {
	        rebuild = true;
	    }
	}
	std::cout << "rebuild (reprocess collection data): " << rebuild << endl; */


    /* get doc vectors
	DocumentRepresentor docRepresentor(colBasePath, laResPath, maxDoc);
	docRepresentor.represent();

	ExplicitSemanticInterpreter esInter(wikiIndexdir, docRepPath);
	esInter.interpret();
	//*/

	//* all pairs similarity search
	boost::shared_ptr<DataSetIterator> dataSetIterator(new SparseVectorSetIterator());
	boost::shared_ptr<AllPairsOutput> output(new AllPairsOutput());

	AllPairsSearch allPairs(output);
	allPairs.findAllSimilarPairs(dataSetIterator);

	//*/

	/*
	DocumentSimilarity DocSimilarity(
	        wikiIndexdir, // esa resource(wiki) path
			laResPath,  // la resource(cma) path
			colBasePath, // collection base path, documents set  ==> using index data ?
			colsspPath, // collection data processing path
			docSimPath,  // data path for document similarity index
			thresholdSim, // similarity threshold value
			maxDoc,      // max documents of collection to be processed
			rebuild // if rebuild collection ssp
			);
	DocSimilarity.DoSim(); //*/

	/*
    DocumentSimilarity DocSimilarity(
            wikiIndexdir, // esa resource(wiki index) path
            laResPath,  // la resource(cma) path
            colBasePath, // collection base path, documents set  ==> using index data
            docSimPath,  // data path for document similarity index
            thresholdSim, // similarity threshold value
            maxDoc      // max documents of collection to be processed
            );
	DocSimilarity.computeSimilarity(); */

	return 0;
}
