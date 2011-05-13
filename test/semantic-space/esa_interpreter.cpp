/**
 * @file esa_interpreter.cpp
 * @author Zhongxia Li
 * @date Apr 6, 2011
 * @brief Semantic Interpreter for ESA
 */

#include <boost/shared_ptr.hpp>
#include <boost/program_options.hpp>

#include <idmlib/semantic_space/term_doc_matrix_defs.h>
#include <idmlib/semantic_space/semantic_space.h>
#include <idmlib/semantic_space/explicit_semantic_space.h>
//#include <idmlib/semantic_space/document_vector_space.h>
#include <idmlib/semantic_space/explicit_semantic_interpreter.h>

namespace po = boost::program_options;
using namespace idmlib::ssp;
//using namespace idmlib::sim;

int main(int argc, char** argv)
{
    string esasspPath; // resource data(Wiki) for explicit semantic analysis
    string laResPath;  // LA (CMA) resource path
    weight_t thresholdSim = 0.0001; // similarity threshold
    uint32_t maxDoc = MAX_DOC_ID;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,H", "produce help message")
        ("esa-res-path,E", po::value<std::string>(&esasspPath), "resource data (Wiki) directory for explicit semantic analysis.")
        ("la-res-path,L", po::value<std::string>(&laResPath), "LA(CMA) resource path.")
        ("threshold-sim,T", po::value<weight_t>(&thresholdSim), "similarity threshold value.")
        ("max-doc,M", po::value<uint32_t>(&maxDoc), "max doc count that will be processed.")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 0;
    }

    if (esasspPath.empty()) {
        esasspPath = "./esa_cnwiki_all";
    }
    cout << "esa-res-path: " <<  esasspPath << endl;
    if (laResPath.empty()) {
        laResPath = "/home/zhongxia/codebase/icma/db/icwb/utf8";
    }
    cout << "la-res-path: " <<  laResPath << endl;

    std::cout << "threshold-sim: " << thresholdSim << endl;
    std::cout << "max-doc: " << maxDoc << endl;

    // Explicit semantic interpreter initialized with wiki knowledge
    boost::shared_ptr<SemanticSpace> pWikiESSpace(new ExplicitSemanticSpace(esasspPath, SemanticSpace::LOAD));
    // pWikiESSpace->Print();
    boost::shared_ptr<SemanticInterpreter> pEsaInterpreter(new ExplicitSemanticInterpreter(pWikiESSpace));

    ///

    return 0;
}
