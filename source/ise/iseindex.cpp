#include <idmlib/ise/iseindex.hpp>

namespace idmlib{ namespace ise{

IseIndex::IseIndex(const IseOptions& options)
{
    LshIndexType::Parameter param;
    param.range = options.range;
    param.repeat = options.repeat;
    param.W = options.w;
    param.dim = options.dim;
    DefaultRng rng;

    lshIndex_.Init(param, rng, options.ntables);
}

IseIndex::~IseIndex()
{
}

}}
