/**
 * @file doc_sim.cpp
 * @author Zhongxia Li
 * @date Mar 11, 2011
 * @brief Document Similarity Test
 */
#include <iostream>
using namespace std;

#include <boost/shared_ptr.hpp>
#include <boost/program_options.hpp>

#include <idmlib/semantic_space/semantic_space.h>
#include <idmlib/semantic_space/explicit_semantic_space.h>
#include <idmlib/semantic_space/semantic_interpreter.h>
#include <idmlib/semantic_space/kpe_semantic_space_builder.h>
#include <idmlib/semantic_space/la_semantic_space_builder.h>
#include <idmlib/semantic_space/term_doc_matrix_defs.h>
#include <idmlib/similarity/document_similarity.h>

namespace po = boost::program_options;
using namespace idmlib::ssp;
using namespace idmlib::sim;

int main(int argc, char** argv)
{
	// argv
	string colPath, outPath, laRecPath;
	uint32_t maxDoc = MAX_DOC_ID;

	po::options_description desc("Allowed options");
	desc.add_options()
		("help,H", "produce help message")
		("col-path,S", po::value<std::string>(&colPath), "base directory of collection.")
		("output-file,O", po::value<std::string>(&outPath), "output file.")
		("la-res-path,L", po::value<std::string>(&laRecPath), "LA(CMA) resource path.")
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

	if (colPath.empty()) {
		colPath = "/home/zhongxia/codebase/sf1-revolution-dev/bin/collection/chinese-wiki-test";
	}
	if (laRecPath.empty()) {
		laRecPath = "/home/zhongxia/codebase/icma/db/icwb/utf8";
	}
	if (outPath.empty()) {
		outPath = "./output";
	}

	if (vm.count("max-doc")) {
		std::cout << "max-doc: " << maxDoc << endl;
	}

	boost::shared_ptr<SemanticSpace> pSSpace;
	pSSpace.reset(new ExplicitSemanticSpace());

	boost::shared_ptr<SemanticSpaceBuilder> pSemBuilder;
	pSemBuilder.reset(new LaSemanticSpaceBuilder(colPath, outPath, pSSpace, laRecPath, maxDoc));
	// 1. build knowledge matrix base on Chinese Wiki
	pSemBuilder->Build();

	// compute document similarity
	//string docSimDir;
	//docSimDir = "./doc-sim";
	//DocumentSimilarity DocSim(pSSP, docSimDir);
	//DocSim.buildSimIndex();

	return 0;
}
