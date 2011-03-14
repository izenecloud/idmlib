/**
 * @file doc_sim.cpp
 * @author Zhongxia Li
 * @date Mar 11, 2011
 * @brief Document Similarity Test
 */
#include <iostream>
using namespace std;

#include <boost/shared_ptr.hpp>

#include <idmlib/semantic_space/semantic_space.h>
#include <idmlib/semantic_space/explicit_semantic_space.h>
#include <idmlib/semantic_space/semantic_interpreter.h>
#include <idmlib/semantic_space/kpe_semantic_space_builder.h>
#include <idmlib/similarity/document_similarity.h>
using namespace idmlib::ssp;
using namespace idmlib::sim;

int main(int argc, char** argv)
{
	cout << "Document Similarity Test." << endl;

	// argv
	string scdPath, outPath;
	scdPath = "/home/zhongxia/codebase/sf1-revolution-dev/bin/collection/chinese-wiki-test/scd/index";
	outPath = "./output";

	boost::shared_ptr<SemanticSpace> pSSpace;
	pSSpace.reset(new ExplicitSemanticSpace());

	boost::shared_ptr<SemanticSpaceBuilder> pSemBuilder;
	pSemBuilder.reset(new KpeSemanticSpaceBuilder(scdPath, outPath, pSSpace));
	// 1. build knowledge matrix
	pSemBuilder->buildInvertedIndex();

	// compute document similarity
	//string docSimDir;
	//docSimDir = "./doc-sim";
	//DocumentSimilarity DocSim(pSSP, docSimDir);
	//DocSim.buildSimIndex();

	return 0;
}
