#include <idmlib/itemset/fptree.h>

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/random.hpp>

#include <list>
#include <vector>
#include <algorithm>
#include <string>

using namespace std;
using namespace boost;
using namespace idmlib;

namespace bfs = boost::filesystem;

void createInput(int maxItem, int recordsNum, const string& output_path)
{
    boost::uniform_int<> distribution(0, maxItem) ;
    boost::mt19937 engine ;
    boost::variate_generator<mt19937, uniform_int<> > myrandom (engine, distribution);

    FILE* output_file = fopen(output_path.c_str(), "wb");

    for(int i = 0; i < recordsNum; ++i)
    {
        std::vector<uint32_t> items;
        for(int j = 0; j < 10; ++j)
        {
            items.push_back(myrandom());
        }
        std::sort(items.begin(), items.end());
        fwrite(&i, sizeof(uint32_t), 1, output_file); // recordId
        uint32_t size = items.size();
        fwrite(&size, sizeof(uint32_t), 1, output_file); // size
        uint32_t* data = &items[0];
        fwrite(data, sizeof(uint32_t), size, output_file); //itemId
    }

    fclose(output_file);
}

BOOST_AUTO_TEST_SUITE(FPTreeTest)

BOOST_AUTO_TEST_CASE(smokeTest)
{
    {
        FPtree fptree;
        int maxItem = 10;
        int recordsNum = 10000;
        fptree.set_item_num(maxItem);
        FPtree::min_sup = 100;
        createInput(maxItem, recordsNum, "fptree.vsd");
        fptree.run("fptree.vsd", "result.txt");
    }
}

BOOST_AUTO_TEST_SUITE_END() 
