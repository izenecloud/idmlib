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

//#define DOC_SIM_TEST

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
	string wikiIndexdir;
	string laResPath;
	string colBasePath;
	string docSetPath;
	string docSimPath;
	float thresholdSim = 0.5;
	uint32_t maxDoc = 0; // max number of documents to be processed, not limited if 0
	string test;

	po::options_description desc("Allowed options");
	desc.add_options()
		("help,H", "produce help message")
		("wiki-index,W", po::value<std::string>(&wikiIndexdir), "[Res]Wikipedia inverted index directory.")
		("la-resource,L", po::value<std::string>(&laResPath), "[Res]LA(CMA) resource path.")
		("doc-col-path,D", po::value<std::string>(&colBasePath), "[Input]collection to be processed.")
		("doc-set-path,S", po::value<std::string>(&docSetPath), "[Output/tmp]document set output dir.")
		("doc-sim-index,I", po::value<std::string>(&docSimPath), "[Output]document similarity index dir.")
		("threshold-sim,T", po::value<float>(&thresholdSim), "similarity threshold value.")
		("max-doc,M", po::value<uint32_t>(&maxDoc), "max doc count that will be processed, not limited as defualt(0).")
		("test,X", po::value<std::string>(&test), "max doc count that will be processed, not limited as defualt(0).")
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

	string esaDir = "./esa";
	if (vm.count("test")) {
	    esaDir += "_test";
	}

	if (wikiIndexdir.empty()) {
	    wikiIndexdir = esaDir + "/wiki";
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

	if (docSetPath.empty()) {
	    docSetPath = esaDir+"/docset";
	}
	cout << "document set output path: " <<  docSetPath << endl;

	if (docSimPath.empty()) {
		docSimPath = esaDir+"/docsim";
	}
	cout << "document similarity index: " <<  docSimPath << endl;

	std::cout << "threshold-sim: " << thresholdSim << endl;
	std::cout << "max-doc: " << maxDoc << endl;


    /* get doc vectors
	DocumentRepresentor docRepresentor(colBasePath, laResPath, docSetPath, maxDoc);
	docRepresentor.represent();
	//
	ExplicitSemanticInterpreter esInter(wikiIndexdir, docSetPath);
	esInter.interpret(maxDoc);
	//*/

	//* all pairs similarity search
	//string datafile = docSetPath+"/doc_rep.vec";
	string datafile = docSetPath+"/doc_int.vec";
	boost::shared_ptr<DataSetIterator> dataSetIterator(new SparseVectorSetIterator(datafile));
	boost::shared_ptr<DocSimOutput> output(new DocSimOutput(docSimPath));

	AllPairsSearch allPairs(output, thresholdSim);
	allPairs.findAllSimilarPairs(dataSetIterator, maxDoc);
	//*/

	/* test
	std::vector<std::pair<uint32_t, float> > result;

	for (size_t idx =1 ; idx <= 3; idx++) {
        output->getSimilarDocIdScoreList(idx,10,result);

        for (size_t i =0; i <result.size(); i++)
            cout <<"("<<result[i].first<<","<<result[i].second<<") ";
        cout << endl;
	}
	//*/

	return 0;
}
