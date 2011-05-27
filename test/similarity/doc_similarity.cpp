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

#include <idmlib/semantic_space/semantic_space.h>
#include <idmlib/semantic_space/explicit_semantic_space.h>
#include <idmlib/semantic_space/document_vector_space.h>
#include <idmlib/semantic_space/explicit_semantic_interpreter.h>
#include <idmlib/semantic_space/term_doc_matrix_defs.h>
#include <idmlib/similarity/document_similarity.h>
#include <la/LA.h>

namespace po = boost::program_options;
using namespace idmlib::ssp;
using namespace idmlib::sim;

int main(int argc, char** argv)
{
	string wikiIndexdir; // resource data(Wiki) for explicit semantic analysis
	string laResPath;  // LA (CMA) resource path
	string colBasePath; // collection of documents to perform doc-similarity computing
	string colsspPath; // collection document vectors pre-processing
	string docSimPath; // document similarity index path
	weight_t thresholdSim = 0.0001; // similarity threshold value
	uint32_t maxDoc = MAX_DOC_ID; // max number of documents to be processed
	bool rebuild = false;

	po::options_description desc("Allowed options");
	desc.add_options()
		("help,H", "produce help message")
		("esa-res-path,E", po::value<std::string>(&wikiIndexdir), "resource data (Wiki) directory for explicit semantic analysis.")
		("la-res-path,L", po::value<std::string>(&laResPath), "LA(CMA) resource path.")
		("col-base-path,C", po::value<std::string>(&colBasePath), "collection to be processed.")
		("col-ssp-path,S", po::value<std::string>(&colsspPath), "collection semantic space data path.")
		("doc-sim-path,D", po::value<std::string>(&docSimPath), "document similarity index path.")
		("threshold-sim,T", po::value<weight_t>(&thresholdSim), "similarity threshold value.")
		("max-doc,M", po::value<uint32_t>(&maxDoc), "max doc count that will be processed.")
		("rebuild-ssp-data,R", po::value<std::string>(), "whether rebuild collection s space data.")
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
	cout << "esa-res-path: " <<  wikiIndexdir << endl;
	if (laResPath.empty()) {
		laResPath = "/home/zhongxia/codebase/icma/db/icwb/utf8";
	}
	cout << "la-res-path: " <<  laResPath << endl;
	if (colBasePath.empty()) {
		colBasePath = "/home/zhongxia/codebase/sf1-revolution-dev/bin/collection/chinese-wiki-test";
	}
	cout << "col-base-path: " <<  colBasePath << endl;
	if (colsspPath.empty()) {
		colsspPath = "./ssp_col";
	}
	cout << "col-ssp-pathh: " <<  colsspPath << endl;
	if (docSimPath.empty()) {
		docSimPath = "./doc_sim";
	}
	cout << "doc-sim-path: " <<  docSimPath << endl;

	std::cout << "threshold-sim: " << thresholdSim << endl;
	std::cout << "max-doc: " << maxDoc << endl;

	if (vm.count("rebuild-ssp-data")) {
	    if (vm["rebuild-ssp-data"].as<std::string>() == "true" || vm["rebuild-ssp-data"].as<std::string>() == "t") {
	        rebuild = true;
	    }
	}
	std::cout << "rebuild (reprocess collection data): " << rebuild << endl;

	// Mining manager ?

	/* deprecated
	DocumentSimilarity DocSimilarity(
			esasspPath, // esa resource(wiki) path
			laResPath,  // la resource(cma) path
			colBasePath, // collection base path, documents set  ==> using index data ?
			colsspPath, // collection data processing path
			docSimPath,  // data path for document similarity index
			thresholdSim, // similarity threshold value
			maxDoc,      // max documents of collection to be processed
			rebuild // if rebuild collection ssp
			);
	DocSimilarity.DoSim(); */

    DocumentSimilarity DocSimilarity(
            wikiIndexdir, // esa resource(wiki index) path
            laResPath,  // la resource(cma) path
            colBasePath, // collection base path, documents set  ==> using index data
            docSimPath,  // data path for document similarity index
            thresholdSim, // similarity threshold value
            maxDoc      // max documents of collection to be processed
            );
	DocSimilarity.computeSimilarity();

	return 0;
}
