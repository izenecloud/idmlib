#include <idmlib/wiki/category_extractor.h>
using namespace idmlib::wiki;

int main(int argc, char** argv)
{
  CategoryExtractor::extract(argv[1], argv[2]);
}
