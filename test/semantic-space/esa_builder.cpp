/**
 * @file esa_builder.cpp
 * @author Zhongxia Li
 * @date Mar 21, 2011
 * @brief Explicit Semantic Analysis
 *
 * Evgeniy Gabrilovich and Shaul Markovitch. (2007).
 * "Computing Semantic Relatedness using Wikipedia-based Explicit Semantic Analysis,"
 * Proceedings of The 20th International Joint Conference on Artificial Intelligence
 * (IJCAI), Hyderabad, India, January 2007.
 */

#include <iostream>
#include <ctime>
using namespace std;

#include <boost/shared_ptr.hpp>
#include <boost/program_options.hpp>
#include <idmlib/semantic_space/semantic_space.h>
#include <idmlib/semantic_space/explicit_semantic_space.h>
#include <idmlib/semantic_space/explicit_semantic_interpreter.h>
#include <idmlib/semantic_space/term_doc_matrix_defs.h>
#include <idmlib/similarity/document_similarity.h>
#include <idmlib/similarity/term_similarity.h>
#include <la/LA.h>
#include <am/matrix/matrix_file_io.h>
#include <am/matrix/sparse_vector.h>

namespace po = boost::program_options;
using namespace idmlib::ssp;
using namespace idmlib::sim;

int main(int argc, char** argv)
{
	string colPath, sspDataPath, laResPath;
	uint32_t maxDoc = MAX_DOC_ID;

	po::options_description desc("Allowed options");
	desc.add_options()
		("help,H", "produce help message")
		("wiki-path,W", po::value<std::string>(&colPath), "base directory of wiki collection.")
		("sem-data-path,S", po::value<std::string>(&sspDataPath), "semantic data path.")
		("la-res-path,L", po::value<std::string>(&laResPath), "LA(CMA) resource path.")
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
	if (laResPath.empty()) {
		laResPath = "/home/zhongxia/codebase/icma/db/icwb/utf8";
	}
	if (sspDataPath.empty()) {
		sspDataPath = "./esa_wiki";
	}

	if (vm.count("max-doc")) {
		std::cout << "max-doc: " << maxDoc << endl;
	}

//  matrix io
//
//	term_doc_matrix* om = new term_doc_matrix(sspDataPath);
//	om->Open();
//	for (size_t i = 0; i < 100; i ++) {
//		doc_sp_vector v;
//		for ( size_t j = 1; j < 5; j ++) {
//			v.value.push_back(std::make_pair(j, 0.3 * i * j));
//		}
//		om->SetVector(i, v);
//	}
//	om->Flush();
//	delete om;
//
//	term_doc_matrix im(sspDataPath);
//	im.Open();
//
//	for (size_t i = 1; i < 10; i += 2) {
//		doc_sp_vector v;
//		im.GetVector(i, v);
//		idmlib::ssp::PrintSparseVec(v);
//	}
//
//	return 0;

//	ExplicitSemanticSpace essp(sspDataPath, SemanticSpace::LOAD);
//	essp.Print();
//	return 0;

	idmlib::ssp::TimeChecker timer("build wiki index");

	// Build knowledge matrix base on Chinese Wiki
	boost::shared_ptr<SemanticSpace> pSSpace( new ExplicitSemanticSpace(sspDataPath) );
	timer.StartPoint();
	boost::shared_ptr<SemanticSpaceBuilder> pSemBuilder(
			new SemanticSpaceBuilder(pSSpace, laResPath, colPath, maxDoc) );
	pSemBuilder->Build();
	pSSpace->Print();
	timer.EndPoint();
	//timer.Print();

/*
	// maps fragments of natural language text into
	// a weighted sequence of Wikipedia concepts ordered by their
	// relevance to the input
	boost::shared_ptr<SemanticInterpreter> pSemInter( new SemanticInterpreter(pSSpace, pSemBuilder) );
	std::string text =
	"但有些情形例外:假如球员打出双杀打而得分、打击时因守备失误而得分、投手暴投或捕手捕逸而得分、或者因投手犯规而得分，他没拿到打点；\
	假如球员被四坏保送或触身球而上垒、打出高飞牺牲打使垒上跑者得分，他就得到打点。此外，假如球员打出全垒打而垒包有两个人，打点则计为3分打点。（因为打击者和两名跑者都得分）";
	UString utext(text, UString::UTF_8);
	std::vector<weight_t> vec;
	pSemInter->interpret(utext, vec);

	cout << "interpretation vector: \n [ ";
	for (size_t t = 0; t < vec.size(); t ++)
	{
		cout << vec[t] << ", " ;
	}
	cout << "]" << endl;

	////
	//boost::shared_ptr<SemanticSpace> pSSpace( new ExplicitSemanticSpace(sspDataPath) );
	//boost::shared_ptr<SemanticSpaceBuilder> pSemBuilder( new LaSemanticSpaceBuilder(colPath, pSSpace, laResPath, maxDoc) );
	//boost::shared_ptr<SemanticInterpreter> pSemInter( new SemanticInterpreter(pSSpace, pSemBuilder) );

	DocumentSimilarity ColDocSim(colPath, pSemInter);
	ColDocSim.Compute();
*/
	return 0;
}
