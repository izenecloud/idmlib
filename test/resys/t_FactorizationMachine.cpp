#include <idmlib/resys/context/FactorizationMachine.h>

#include <am/graphchi/engine/dynamic_graphs/graphchi_dynamicgraph_engine.hpp>

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/random.hpp>

#include <util/ClockTimer.h>
#include <boost/timer.hpp>

#include <list>
#include <string>

#include <cmath>


using namespace graphchi;
using namespace idmlib::recommender;

namespace bfs = boost::filesystem;

const char* TEST_DIR_STR = "fm";

BOOST_AUTO_TEST_SUITE(FactorizationMachineTest)

BOOST_AUTO_TEST_CASE(factorInmemTest)
{
    bfs::path fmPath(TEST_DIR_STR);
    boost::filesystem::remove_all(fmPath);
    bfs::create_directories(fmPath);

    std::string filename = fmPath.string() + "/fmdb";
    typedef vertex_data<20> VertexDataType;
    typedef FactorContainerInMem<VertexDataType> FactorContainerType;
    unsigned size = 100;
    {
        FactorContainerType db(filename);
        db.Init(size);
        for(unsigned i = 0; i < size; ++i)
        {
            VertexDataType& v = db[i];
            v.bias = i;
        }
    }
    FactorContainerType db(filename);	
    db.Init(size);
    for(unsigned i = 0; i < size; ++i)
    {
        VertexDataType& v = db[i];
        BOOST_CHECK_EQUAL(v.bias, i);
    }
}

BOOST_AUTO_TEST_CASE(factorDBTest)
{
    bfs::path fmPath(TEST_DIR_STR);
    boost::filesystem::remove_all(fmPath);
    bfs::create_directories(fmPath);

    std::string filename = fmPath.string() + "/fmdb";
    typedef vertex_data<20> VertexDataType;
    typedef FactorContainerDB<VertexDataType> FactorContainerType;
    unsigned size = 100;
    {
        FactorContainerType db(filename, sizeof(VertexDataType)*10);
        db.Init(size);
        for(unsigned i = 0; i < size; ++i)
        {
            VertexDataType& v = db[i];
            v.bias = i;
        }
    }
    FactorContainerType db(filename, sizeof(VertexDataType)*10);
    db.Init(size);
    for(unsigned i = 0; i < size; ++i)
    {
        VertexDataType& v = db[i];
        BOOST_CHECK_EQUAL(v.bias, i);
    }
}


BOOST_AUTO_TEST_CASE(smokeTest)
{
#if 0
    bfs::path fmPath(TEST_DIR_STR);
    boost::filesystem::remove_all(fmPath);
    bfs::create_directories(fmPath);

    metrics m("fm");

    typedef vertex_data<20> VertexDataType;
    typedef edge_data<0> EdgeDataType;

    typedef FactorizationMachine<20, VertexDataType, EdgeDataType > FMType;
    typedef FactorizationMachine<20, VertexDataType, EdgeDataType, true > ImplicitFMType;

    std::string filename = fmPath.string() + "/graphchi_fm";
    
    int niters = 4; // Number of iterations
    bool scheduler = false;                       // Whether to use selective scheduling

    /* Detect the number of shards or preprocess an input to creae them */
    int nshards = convert_if_notexists<EdgeDataType >(filename, 
                                                              get_option_string("nshards", "auto"));

    /* Run */
    graphchi_engine<VertexDataType, EdgeDataType > engine(filename, nshards, scheduler, m); 
    engine.set_disable_vertexdata_storage();  
    engine.set_modifies_outedges(false);
    engine.set_modifies_inedges(false); // Improves I/O performance.
    std::string factor_db = filename + "/fmdb";
    FMType fm(factor_db,1,0.1,0.1,0.1);
    engine.run(fm, niters);
    ImplicitFMType fm2(factor_db,1,0.1,0.1,0.1);

    metrics_report(m);
#endif
}

BOOST_AUTO_TEST_SUITE_END()
