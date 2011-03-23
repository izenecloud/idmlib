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
#include <idmlib/semantic_space/semantic_interpreter.h>
#include <idmlib/semantic_space/kpe_semantic_space_builder.h>
#include <idmlib/semantic_space/la_semantic_space_builder.h>
#include <idmlib/semantic_space/term_doc_matrix_defs.h>
#include <idmlib/similarity/document_similarity.h>
#include <la/LA.h>

namespace po = boost::program_options;
using namespace idmlib::ssp;
using namespace idmlib::sim;

int main(int argc, char** argv)
{
	string colPath, sspDataPath, laResPath;
	count_t maxDoc;

	boost::shared_ptr<SemanticSpace> pSSpace( new ExplicitSemanticSpace(sspDataPath) );
	boost::shared_ptr<SemanticSpaceBuilder> pSemBuilder( new LaSemanticSpaceBuilder(colPath, pSSpace, laResPath, maxDoc) );
	boost::shared_ptr<SemanticInterpreter> pSemInter( new SemanticInterpreter(pSSpace, pSemBuilder) );

	DocumentSimilarity ColDocSim(colPath, pSemInter);
	ColDocSim.Compute();

	return 0;
}
