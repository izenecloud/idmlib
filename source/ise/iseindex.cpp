#include <idmlib/ise/iseindex.hpp>

#include <boost/filesystem.hpp>

namespace bfs = boost::filesystem;

namespace idmlib{ namespace ise{

IseIndex::IseIndex(const std::string& homePath)
    :lshHome_(bfs::path(bfs::path(homePath)/"lshindex").string())
    ,imgMetaHome_(bfs::path(bfs::path(homePath)/"imgmeta").string())
    ,imgMetaStorage_(imgMetaHome_)
{
    bfs::create_directories(homePath);
    if (!imgMetaStorage_.open())
    {
        bfs::remove_all(imgMetaHome_);
        imgMetaStorage_.open();
    }
	
    if(bfs::exists(lshHome_))
    {
        std::ifstream is(lshHome_.c_str(), std::ios_base::binary);
        lshIndex_.Load(is);
    }
}

IseIndex::~IseIndex()
{
    imgMetaStorage_.close();
}

void IseIndex::ResetLSH(const IseOptions& options)
{
    bfs::remove_all(lshHome_);

    LshIndexType::Parameter param;
    param.range = options.range;
    param.repeat = options.repeat;
    param.W = options.w;
    param.dim = options.dim;
    DefaultRng rng;
    lshIndex_.Init(param, rng, options.ntables);
}

void IseIndex::SaveLSH()
{
    std::ofstream os(lshHome_.c_str(), std::ios_base::binary);
    lshIndex_.Save(os);
}

void IseIndex::Insert(const std::string& imgPath)
{
    static unsigned id = 0;
    ++id;
    std::vector<Sift::Feature> sift;
    extractor_.ExtractSift(imgPath, sift, false);
    std::vector<Sketch > sketches;
    extractor_.BuildSketch(sift, sketches);
    for(unsigned i = 0; i < sift.size(); ++i)
        lshIndex_.Insert(&sift[i].desc[0],id);
    imgMetaStorage_.insert(id,imgPath);
}

void IseIndex::Search(const std::string& queryImgPath, std::vector<std::string>& results)
{
    std::vector<Sift::Feature> sift;
    extractor_.ExtractSift(queryImgPath, sift, false);
    std::vector<unsigned> imgIds;
    for(unsigned i = 0; i < sift.size(); ++i)
    {
        lshIndex_.Search(&sift[i].desc[0], imgIds);
    }
    for(unsigned i = 0; i < imgIds.size(); ++i)
    {
        std::string img;
        if(imgMetaStorage_.get(imgIds[i],img))
            results.push_back(img);
    }
}

}}
