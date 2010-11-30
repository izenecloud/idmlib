#include <idmlib/util/resource_util.h>
#include <boost/program_options.hpp>

using namespace idmlib;
using namespace idmlib::util;
namespace po = boost::program_options;



int main(int ac, char** av)
{
  po::options_description desc("Allowed options");
  desc.add_options()
    ("help", "produce help message")
    ("file,F", po::value<std::string>(), "resource file location")
  ;
  po::variables_map vm;
  po::store(po::parse_command_line(ac, av, desc), vm);
  po::notify(vm); 
  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return 1;
  }
  std::string file;
  if (vm.count("file")) {
    file = vm["file"].as<std::string>();
    std::cout << "file: " << file <<std::endl;
  } 
  else {
    std::cerr<<"file not set"<<std::endl;
    std::cerr << desc << std::endl;
    return -1;
  }
  bool succ = idmlib::util::generateResource(file);
  if(!succ)
  {
    std::cerr<<"generate resource failed"<<std::endl;
    return -1;
  }
  
  
  return 0;

  
}
