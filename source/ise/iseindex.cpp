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

void IseIndex::Insert(const std::string& imgPath)
{
    static unsigned id = 0;
    std::vector<Sift::Feature> sift;
    extractor_.ExtractSift(imgPath, sift, false);
    std::cout<<"sift size "<<sift.size()<<std::endl;
    std::vector<Sketch > sketches;
    extractor_.BuildSketch(sift, sketches);
    for(unsigned i = 0; i < sift.size(); ++i)
        lshIndex_.Insert(&sift[i].desc[0],id++);
}

}}
