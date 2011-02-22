#include <boost/regex.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <fstream>
void filter_xingzhengquhua(const std::string& file)
{
  std::ifstream ifs(file.c_str());
  std::ofstream ofs( (file+".filtered").c_str());
  std::string line;
  while(getline( ifs, line))
  {
    if( line.find("行政区划") != std::string::npos)
    {
      continue;
    }
    if( boost::starts_with( line, "省" ))
    {
      continue;
    }
    if( boost::starts_with( line, "街道" ))
    {
      continue;
    }
    if( boost::starts_with( line, "区" ))
    {
      continue;
    }
    if( boost::starts_with( line, "县" ))
    {
      continue;
    }
    if( boost::starts_with( line, "市" ))
    {
      continue;
    }
    if( boost::starts_with( line, "镇" ))
    {
      continue;
    }
    if( boost::starts_with( line, "乡" ))
    {
      continue;
    }
    if( boost::starts_with( line, "道" ))
    {
      continue;
    }
    if( boost::ends_with( line, "代码" ))
    {
      continue;
    }
    ofs<<line<<std::endl;
  }
  ifs.close();
  ofs.close();
}


int main(int argc, char** argv)
{
  filter_xingzhengquhua(argv[1]);
  return 0;

}
