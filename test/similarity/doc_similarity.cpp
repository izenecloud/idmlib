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
	string esasspPath; // resource data(Wiki) for explicit semantic analysis
	string laResPath;  // LA (CMA) resource path
	string colBasePath; // collection of documents to perform doc-similarity computing
	string colsspPath; // collection document vectors pre-processing
	count_t maxDoc; // max number of documents to be processed

	po::options_description desc("Allowed options");
	desc.add_options()
		("help,H", "produce help message")
		("esa-res-path,E", po::value<std::string>(&esasspPath), "resource data (Wiki) directory for explicit semantic analysis.")
		("la-res-path,L", po::value<std::string>(&laResPath), "LA(CMA) resource path.")
		("col-base-path,C", po::value<std::string>(&colBasePath), "collection to be processed.")
		("col-ssp-path,S", po::value<std::string>(&colsspPath), "collection semantic space data path.")
		("max-doc,M", po::value<uint32_t>(&maxDoc), "max doc count that will be processed.")
	;
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if (vm.count("help")) {
		std::cout << desc << std::endl;
		return 0;
	}
	if (vm.empty()) {
		std::cout << desc << std::endl;
		std::cout << "" << std::endl << std::endl;
	}

	if (esasspPath.empty()) {
		esasspPath = "./esa_wiki";
	}
	if (laResPath.empty()) {
		laResPath = "/home/zhongxia/codebase/icma/db/icwb/utf8";
	}
	if (colBasePath.empty()) {
		colBasePath = "/home/zhongxia/codebase/sf1-revolution-dev/bin/collection/chinese-wiki-test";
	}
	if (colsspPath.empty()) {
		colsspPath = "./ssp_col";
	}

	if (vm.count("max-doc")) {
		std::cout << "max-doc: " << maxDoc << endl;
	}

	//////////////////////////

	/* Explicit semantic interpreter initialized with wiki knowledge */
	boost::shared_ptr<SemanticSpace> pWikiESSpace(new ExplicitSemanticSpace(esasspPath, SemanticSpace::LOAD));
	boost::shared_ptr<SemanticInterpreter> pESInter(new ExplicitSemanticInterpreter(pWikiESSpace));
	//pWikiESSpace->Print();

	// document similarity for documents of collection
	/* pre-process for collection data */
	boost::shared_ptr<SemanticSpace> pDocVecSpace(new DocumentVectorSpace(colsspPath));
	boost::shared_ptr<SemanticSpaceBuilder> pCollectionBuilder(new SemanticSpaceBuilder(pDocVecSpace, laResPath, colBasePath, maxDoc));
	pCollectionBuilder->Build();

	//pDocVecSpace->Print();

	/* interpret all */
	pESInter->interpret(pDocVecSpace);


	// for docVec in collection
	//     interVec = interpret(docVec)
	//     addtoInvertedIndex(interVec) //


	//DocumentSimilarity DocSim(esasspPath, colPath, pSemInter);
	//DocSim.Compute();
	//DocSim.ComputeAll(pDocVecSpace);

	return 0;
}
