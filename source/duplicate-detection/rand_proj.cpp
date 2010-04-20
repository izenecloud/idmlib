/**
   @file rand_proj.cpp
   @author Kevin Hu
   @date 2009.11.24
 */
#include <idmlib/duplicate-detection/rand_proj.h>

using namespace std;
namespace sf1v5{

void RandProj::operator+=(const RandProj& rp)
{
    for (uint32_t i = 0; i < proj_.length(); i++) {
      proj_[i] += rp.at(i);
    }
}

void RandProj::generate_bitarray(wiselib::CBitArray& bitArray)
{
    bitArray.ResetAll();
    for (uint32_t i = 0; i < proj_.length(); i++) {
      if (proj_.at(i) >= 0) {
            bitArray.SetAt(i);
        }
    }
}

}
