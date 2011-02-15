#include <idmlib/wiki/features_gen.h>
#include <boost/regex.hpp>
using namespace idmlib::wiki;

int main(int argc, char** argv)
{

  FeaturesGen gen(argv[1]);
  gen.gen_all(argv[2]);
}
