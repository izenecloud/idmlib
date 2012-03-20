#include <idmlib/ise/iseindex.hpp>

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>

#include <vector>
#include <string>
#include <iostream>
#include <fstream>

namespace po = boost::program_options;
namespace bfs = boost::filesystem;

int main(int argc, char **argv)
{
    std::string input;
    po::options_description desc("Allowed options");
    desc.add_options()
    ("input,F", po::value(&input), "image directory")
    ("query,Q", "query image")
    ;

    po::positional_options_description p;

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).
                     options(desc).positional(p).run(), vm);
    po::notify(vm);

    if ((vm.count("input") == 0)&&(vm.count("query") == 0) ) {
        std::cerr << desc;
        return 1;
    }

    idmlib::ise::IseIndex iseIndex("ise");

    if(vm.count("input") != 0)
    {
        idmlib::ise::IseOptions options;
        options.range = 1000000;
        options.repeat = 100;
        options.w = 8.0F;
        options.dim = 128;
        options.ntables = 4;
        iseIndex.Reset(options);
        bfs::recursive_directory_iterator dir_iter(input), end_iter;
        for(; dir_iter!= end_iter; ++dir_iter)
        {
            if(bfs::is_regular_file(*dir_iter))
            {
                iseIndex.Insert(bfs::path(*dir_iter).string());
            }
        }
        iseIndex.Save();
    }
    else if(vm.count("query") !=0)
    {
        for(;;)
        {
            std::string queryImgPath;
            std::cin >> queryImgPath;
            if(!std::cin) break;
            std::vector<std::string> results;
            iseIndex.Search(queryImgPath, results);
            for(unsigned i = 0; i < results.size(); ++i)
                std::cout<<results[i]<<std::endl;
            std::cout<<"result size: "<<results.size()<<std::endl;
        }
    }
    return 0;
}
