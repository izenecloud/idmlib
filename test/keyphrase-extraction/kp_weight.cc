#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

using namespace boost::filesystem;
namespace po = boost::program_options;

int main(int ac, char** av)
{
  po::options_description desc("Allowed options");
  desc.add_options()
    ("help", "produce help message")
    ("file,F", po::value<std::string>(), "kp list file")
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
  std::string output_file = file+".weighted";
  std::ifstream ifs(file.c_str());
  std::string line;
  std::vector<std::pair<double, std::string> > items;
  while(getline(ifs, line))
  {
    std::vector<std::string> vec;
    boost::algorithm::split( vec, line, boost::algorithm::is_any_of(",") );
    if(vec.size()<3)
    {
      std::cout<<"invalid line : "<<line<<std::endl;
      continue;
    }
    std::string text = vec[0];
    uint32_t freq = boost::lexical_cast<uint32_t>(vec[1]);
    uint32_t df = boost::lexical_cast<uint32_t>(vec[2]);
    double value = (double)freq/std::log((double)(df+1));
    items.push_back(std::make_pair(value, text));
  }
  ifs.close();
  std::sort(items.begin(), items.end(), std::greater<std::pair<double, std::string> >());
  std::ofstream ofs(output_file.c_str());
  for(uint32_t i=0;i<items.size();i++)
  {
    ofs<<items[i].second<<"\t"<<items[i].first<<std::endl;
  }
  ofs.close();
  return 0;

  
}
