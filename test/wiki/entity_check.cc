#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
int main(int argc, char** argv)
{
  std::string key = argv[1];
  std::string file = argv[2];
  boost::regex reg_exp(key);
  std::ifstream ifs(file.c_str());
  if(ifs.fail())
  {
    std::cout<<"can not find "<<file<<std::endl;
    return false;
  }
  std::string line;
  while( getline(ifs, line) )
  {
    std::vector<std::string> vec;
    boost::algorithm::split( vec, line, boost::algorithm::is_any_of("\t") );
    std::string surface = vec[0];
    std::string ref = vec[1];
    if(boost::regex_match(surface, reg_exp))
    {
      if(ref!=surface)
      {
        std::cout<<surface<<"\t"<<ref<<std::endl;
      }
    }
  }
  ifs.close();
}
