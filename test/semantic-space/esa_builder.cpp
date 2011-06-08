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
//#include <idmlib/similarity/document_similarity.h>

#include <la/LA.h>

#ifndef __SCD__PARSER__H__
#define __SCD__PARSER__H__
#include <util/scd_parser.h>
#endif

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
		("wiki-col-path,W", po::value<std::string>(&wikiColPath), "base directory of wiki collection.")
		("wiki-index-path,I", po::value<std::string>(&wikiIndexDir), "semantic resource data path.")
		("la-res-path,L", po::value<std::string>(&laResPath), "LA(CMA) resource path.")
		("max-doc,M", po::value<uint32_t>(&maxDoc), "max doc count that will be processed.")
		("print,P", po::value<std::string>(), "Print result")
	;
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if (vm.count("help")) {
		std::cout << desc << std::endl;
		return 0;
	}
//	if (vm.empty()) {
//		std::cout << desc << std::endl;
//	}

	if (wikiColPath.empty()) {
	    wikiColPath = "/home/zhongxia/codebase/sf1-revolution-dev/bin/collection/chinese-wiki-test";
	}
	cout << "wiki-collection-path: " << wikiColPath << endl;

	if (laResPath.empty()) {
		laResPath = "/home/zhongxia/codebase/icma/db/icwb/utf8";
	}
	cout << "la-resource-path: " << laResPath << endl;

	if (wikiIndexDir.empty()) {
	    wikiIndexDir = "./cnwiki/index";
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

    boost::shared_ptr<WikiIndex> wikiIndex(new MemWikiIndex());
    WikiIndexBuilder wikiIndexBuilder(wikiColPath, laResPath, maxDoc);
    wikiIndexBuilder.build(wikiIndex);
    wikiIndex->load();

    /* deprecated
    IzeneWikiIndexBuilder izeneWikiIndexBuilder(wikiColPath, laResPath, maxDoc);
    izeneWikiIndexBuilder.build();
    //*/

    return 0;
    /////

	// Build knowledge matrix base on Chinese Wiki
	//*
//	boost::shared_ptr<SemanticSpace> pSSpace( new ExplicitSemanticSpace(wikiIndexDir) );
//
//	boost::shared_ptr<SemanticSpaceBuilder> pSemBuilder(
//			new SemanticSpaceBuilder(pSSpace, laResPath, wikiColPath, maxDoc) );
//	pSemBuilder->Build();
//
//	if (print) {
//        pSSpace->Print();
//    }
	//*/

	/*
    boost::shared_ptr<SemanticSpaceBuilder> pSemBuilder(
            new SemanticSpaceBuilder(wikiIndexDir, laResPath, wikiColPath, maxDoc) );
	pSemBuilder->BuildWikiSource();
    //*/


	return 0;
}

/*
 * void ScdModifier(const string& scdDir, count_t& maxDoc);
 *
/// modify DOCIDs
void ScdModifier(const string& scdDir, count_t& maxDoc)
{
    std::vector<string> scdFiles;
    if (!SemanticSpaceBuilder::getScdFileList(scdDir, scdFiles))
        return;

    cout << "SCDFiles: " << scdFiles.size() << endl;
    for (size_t i = 0; i < scdFiles.size(); i++)
    {
        string outScdFile = scdFiles[i] + ".Modif";
        cout << "Modifying DOCIDs, from " << scdFiles[i] << " to \n" << outScdFile << endl;

        ofstream of(outScdFile.c_str(), ios_base::out);
        if (of.bad()) {
            cout << "failed to open file :" << outScdFile <<endl;
            return ;
        }

        typedef std::vector<std::pair<izenelib::util::UString, izenelib::util::UString> >::iterator doc_properties_iterator;

        izenelib::util::ScdParser scdParser(izenelib::util::UString::UTF_8); // default encoding
        scdParser.load(scdFiles[i]);
        izenelib::util::ScdParser::iterator iter = scdParser.begin();
        docid_t docid = 0; // start at 1
        for ( ; iter != scdParser.end(); iter ++)
        {
            izenelib::util::SCDDocPtr pDoc = *iter;

            doc_properties_iterator proIter;
            for (proIter = pDoc->begin(); proIter != pDoc->end(); proIter ++)
            {
                izenelib::util::UString propertyNameTest = proIter->first;
                string propertyName = la::to_utf8(proIter->first);
                string propertyValue = la::to_utf8(proIter->second);

                propertyNameTest.toLowerString();
                if (propertyNameTest == izenelib::util::UString("docid", izenelib::util::UString::UTF_8) )
                {
                    stringstream ss;
                    ss << (++ docid);
                    propertyValue = ss.str();
                }

                of << "<" << propertyName << ">" << propertyValue << endl;
            }

            if (docid % 1000 == 0) {
                cout << "[docid] "<< docid << endl;
            }
            if (docid >= maxDoc) {
                break;
            }
        }
        cout << "Finished at docid:" << docid <<endl;
        of.close();

        break; // 1 file
    }
}
*/
