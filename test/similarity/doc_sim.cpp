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
#include <idmlib/similarity/document_similarity.h>
using namespace idmlib::ssp;
using namespace idmlib::sim;

int main(int argc, char** argv)
{
	cout << "Document Similarity Test." << endl;

	// argv
	string scdDir;
	boost::shared_ptr<SemanticSpace> pSSP;
	scdDir = "/home/zhongxia/codebase/sf1-revolution-dev/bin/collection/chinese-wiki-test/scd/index";
	pSSP.reset(new ExplicitSemanticSpace(scdDir));

	string docSimDir;
	docSimDir = "./doc-sim";
	DocumentSimilarity DocSim(pSSP, docSimDir);
	DocSim.buildSimIndex();

	return 0;
}
