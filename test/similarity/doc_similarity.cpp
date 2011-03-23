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

	// Explicit semantic interpreter initialized with wiki knowledge
	boost::shared_ptr<SemanticSpace> pWikiESSpace(new ExplicitSemanticSpace(esasspPath));
	boost::shared_ptr<SemanticInterpreter> pESInter(new ExplicitSemanticInterpreter(pWikiESSpace));

	// pre-process for collection data
	boost::shared_ptr<SemanticSpace> pDocVecSpace(new DocumentVectorSpace(colsspPath));
	boost::shared_ptr<SemanticSpaceBuilder> pCollectionBuilder(new SemanticSpaceBuilder(pDocVecSpace, laResPath, colBasePath, maxDoc));
	pCollectionBuilder->Build();

	// for docVec in collection
	//     interVec = interpret(docVec)
	//     addtoInvertedIndex(interVec) //


	//DocumentSimilarity DocSim(esasspPath, colPath, pSemInter);
	//DocSim.Compute();
	//DocSim.ComputeAll(pDocVecSpace);

	return 0;
}
