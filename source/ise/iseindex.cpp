#include <idmlib/ise/iseindex.hpp>

#include <boost/filesystem.hpp>

namespace bfs = boost::filesystem;

namespace idmlib{ namespace ise{

IseIndex::IseIndex(const std::string& homePath, ALGORITHM algo)
    : lshHome_((bfs::path(homePath) / "lshindex").string())
    , imgMetaHome_((bfs::path(homePath) / "imgmeta").string())
    , psmAlgo_((bfs::path(homePath) / "psm").string(), algo == BF)
    , imgMetaStorage_(imgMetaHome_)
    , algo_(algo)
{
    bfs::create_directories(homePath);
    if (!imgMetaStorage_.open())
    {
        bfs::remove_all(imgMetaHome_);
        imgMetaStorage_.open();
    }

    switch (algo_)
    {
    case LSH:
        if (bfs::exists(lshHome_))
        {
            std::ifstream is(lshHome_.c_str(), std::ios_base::binary);
            lshIndex_.Load(is);
        }
        break;

    case BF:
    case PSM:
        psmAlgo_.Init();
        break;

    default:
        break;
    }
}

IseIndex::~IseIndex()
{
    imgMetaStorage_.close();
}

void IseIndex::Reset(const IseOptions& options)
{
    ResetLSH_(options);
}

void IseIndex::Save()
{
    switch (algo_)
    {
    case LSH:
        SaveLSH_();
        break;

    case BF:
    case PSM:
        psmAlgo_.Finish();
        break;

    default:
        break;
    }
}

void IseIndex::ResetLSH_(const IseOptions& options)
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

void IseIndex::SaveLSH_()
{
    std::ofstream os(lshHome_.c_str(), std::ios_base::binary);
    lshIndex_.Save(os);
}

bool IseIndex::Insert(const std::string& imgPath)
{
    static unsigned id = 0;
    ++id;
    std::vector<Sift::Feature> sift;
    extractor_.ExtractSift(imgPath, sift, false);
    if (sift.empty()) return false;
    switch (algo_)
    {
    case LSH:
        for (unsigned i = 0; i < sift.size(); ++i)
            lshIndex_.Insert(&sift[i].desc[0], id);
        break;

    case BF:
    case PSM:
        {
            std::vector<Sketch> sketches;
            extractor_.BuildSketch(sift, sketches);
            psmAlgo_.Insert(id, sketches);
        }
        break;

    default:
        break;
    }
    imgMetaStorage_.insert(id, imgPath);
    return true;
}

void IseIndex::Search(const std::string& queryImgPath, std::vector<std::string>& results)
{
    std::vector<Sift::Feature> sift;
    extractor_.ExtractSift(queryImgPath, sift, true);
    std::vector<unsigned> imgIds;
    switch (algo_)
    {
    case LSH:
        DoLSHSearch_(sift, imgIds);
        break;

    case BF:
    case PSM:
        DoPSMSearch_(sift, imgIds);
        break;

    default:
        break;
    }
    for (unsigned i = 0; i < imgIds.size(); ++i)
    {
        std::string img;
        if (imgMetaStorage_.get(imgIds[i], img))
            results.push_back(img);
    }
}

void IseIndex::DoLSHSearch_(std::vector<Sift::Feature>& sift, std::vector<unsigned>& results)
{
    for (unsigned i = 0; i < sift.size(); ++i)
    {
        lshIndex_.Search(&sift[i].desc[0], results);
    }
    std::sort(results.begin(), results.end());
    results.resize(std::unique(results.begin(), results.end()) - results.begin());
}

void IseIndex::DoPSMSearch_(std::vector<Sift::Feature>& sift, std::vector<unsigned>& results)
{
    std::vector<Sketch> sketches;
    extractor_.BuildSketch(sift, sketches);
    for (unsigned i = 0; i < sketches.size(); ++i)
    {
        psmAlgo_.Search(sketches[i], results);
    }
    std::sort(results.begin(), results.end());
    results.resize(std::unique(results.begin(), results.end()) - results.begin());
}

void IseIndex::DoPostFiltering_(std::vector<unsigned>& in, std::vector<unsigned>& out)
{
}

}}
