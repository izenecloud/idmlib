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
    ;

    po::positional_options_description p;

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).
                     options(desc).positional(p).run(), vm);
    po::notify(vm);

    if ((vm.count("input") == 0) ) {
        std::cerr << desc;
        return 1;
    }

    idmlib::ise::IseOptions options;
    options.range = 1000000;
    options.repeat = 10;
    options.w = 8;
    options.dim = 128;

    idmlib::ise::IseIndex iseIndex(options);
    bfs::recursive_directory_iterator dir_iter(input), end_iter;
    for(; dir_iter!= end_iter; ++dir_iter)
    {
        if(bfs::is_regular_file(*dir_iter))
        {
            std::cout<<bfs::path(*dir_iter).string()<<std::endl;
            iseIndex.Insert(bfs::path(*dir_iter).string());
        }
    }

    return 0;
}
