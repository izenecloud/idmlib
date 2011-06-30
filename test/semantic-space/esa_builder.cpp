/**
 * @file esa_builder.cpp
 * @author Zhongxia Li
 * @date Mar 21, 2011
 * @brief Build inverted index for wikipedia
 *
 * Evgeniy Gabrilovich and Shaul Markovitch. (2007).
 * "Computing Semantic Relatedness using Wikipedia-based Explicit Semantic Analysis,"
 * Proceedings of The 20th International Joint Conference on Artificial Intelligence
 * (IJCAI), Hyderabad, India, January 2007.
 */

#include <fstream>
#include <iostream>
#include <ctime>
using namespace std;

#include <boost/shared_ptr.hpp>
#include <boost/program_options.hpp>

#include <idmlib/semantic_space/esa/WikiIndexBuilder.h>

#include <la/LA.h>

namespace po = boost::program_options;
using namespace idmlib::ssp;



int main(int argc, char** argv)
{
	string wikiColPath, wikiIndexDir, laResPath;
	uint32_t maxDoc = 0;
	bool print = false;

	po::options_description desc("Allowed options");
	desc.add_options()
		("help,H", "produce help message")
		("wiki-col-path,W", po::value<std::string>(&wikiColPath), "[Input]base directory of wiki collection.")
		("wiki-index-path,I", po::value<std::string>(&wikiIndexDir), "[Output]Wikipedia index data path.")
		("la-res-path,L", po::value<std::string>(&laResPath), "LA(CMA) resource path.")
		("max-doc,M", po::value<uint32_t>(&maxDoc), "max doc count that will be processed, not limited as default (0).")
		("print,P", po::value<std::string>(), "Print result")
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
	}

	if (wikiColPath.empty()) {
	    wikiColPath = "/home/zhongxia/codebase/sf1r-engine/bin/collection/chinese-wiki-test";
	}
	cout << "wiki-collection-path: " << wikiColPath << endl;

	if (laResPath.empty()) {
		laResPath = "/home/zhongxia/codebase/icma/db/icwb/utf8";
	}
	cout << "la-resource-path: " << laResPath << endl;

	if (wikiIndexDir.empty()) {
	    wikiIndexDir = "./esa_test/wiki";
	}
	cout << "wiki-index-data-path: " << wikiIndexDir << endl;

	if (vm.count("max-doc")) {
        ;
	}
	std::cout << "max-doc: " << maxDoc << endl;

    if (vm.count("print")) {
            print = true;
    }
    std::cout << "print: " << print << endl;

    std::vector<std::string> processProperties;
    processProperties.push_back("Title");
    processProperties.push_back("Content");

    //*
    WikiIndexBuilder wikiIndexBuilder(wikiColPath, laResPath, processProperties, maxDoc);
    boost::shared_ptr<MemWikiIndex> wikiIndex(new MemWikiIndex(wikiIndexDir));
    wikiIndexBuilder.build(wikiIndex);
    //wikiIndex->load(); // test */

    /* xxx
    IzeneWikiIndexBuilder izeneWikiIndexBuilder(wikiColPath, laResPath, processProperties, maxDoc);
    izeneWikiIndexBuilder.build();
    //*/

    return 0;
}


