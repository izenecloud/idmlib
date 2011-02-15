#include <idmlib/wiki/entity_extractor.h>
using namespace idmlib::wiki;

int main(int argc, char** argv)
{
  EntityExtractor::extract(argv[1], argv[2]);
}