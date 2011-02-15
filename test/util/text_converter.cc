#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <util/ustring/UString.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <idmlib/util/idm_analyzer.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>


int main(int argc, char** argv)
{
  izenelib::am::rde_hash<izenelib::util::UCS2Char, izenelib::util::UCS2Char> map;
  {
    std::string file = "./simpler";
    std::ifstream ifs(file.c_str());
    if(ifs.fail())
    {
      std::cout<<"can not find "<<file<<std::endl;
      return -1;
    }
    std::string line;
    while( getline( ifs, line) )
    {
      boost::algorithm::trim( line );
      izenelib::util::UString uline(line, izenelib::util::UString::UTF_8);
      map.insert( uline[2], uline[0] );
    }
    ifs.close();
  }
  {
    std::string file = argv[1];
    std::string output_file = file+".simple";
    std::ifstream ifs(file.c_str());
    std::ofstream ofs(output_file.c_str());
    if(ifs.fail())
    {
      std::cout<<"can not find "<<file<<std::endl;
      return -1;
    }
    std::string line;
    while( getline( ifs, line) )
    {
      boost::algorithm::trim( line );
      izenelib::util::UString uline(line, izenelib::util::UString::UTF_8);
      for(uint32_t i=0;i<uline.length();i++)
      {
        izenelib::util::UCS2Char* p_char = map.find(uline[i]);
        if( p_char != NULL )
        {
          uline[i] = *p_char;
        }
      }
      std::string str;
      uline.convertString(str, izenelib::util::UString::UTF_8);
      ofs<<str<<std::endl;
    }
    ifs.close();
    ofs.close();
  }

}
