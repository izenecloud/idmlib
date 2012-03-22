#include <idmlib/ise/iseindex.hpp>

#include <boost/filesystem.hpp>

namespace bfs = boost::filesystem;

namespace idmlib{ namespace ise{

IseIndex::IseIndex(const std::string& homePath, ALGORITHM algo)
    : lshHome_((bfs::path(homePath) / "lshindex").string())
    , imgMetaHome_((bfs::path(homePath) / "imgmeta").string())
    , bruteForce_((bfs::path(homePath) / "sketchindex").string())
    , probSimMatch_((bfs::path(homePath) / "psmindex").string())
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

    case BF_SKETCH:
        bruteForce_.Init();
        break;

    case PSM:
    case BF_SIMHASH:
        probSimMatch_.Init();
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

    case BF_SKETCH:
        bruteForce_.Finish();
        break;

    case PSM:
    case BF_SIMHASH:
        probSimMatch_.Finish();
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
    std::vector<Sift::Feature> sifts;
    extractor_.ExtractSift(imgPath, sifts, false);
    if (sifts.empty()) return false;
    switch (algo_)
    {
    case LSH:
        for (unsigned i = 0; i < sifts.size(); ++i)
            lshIndex_.Insert(&sifts[i].desc[0], id);
        break;

    case BF_SKETCH:
        {
            std::vector<Sketch> sketches;
            extractor_.BuildSketch(sifts, sketches);
            bruteForce_.Insert(id, sketches);
        }
        break;

    case PSM:
    case BF_SIMHASH:
        probSimMatch_.Insert(id, sifts);
        break;

    default:
        break;
    }
    imgMetaStorage_.insert(id, imgPath);
    return true;
}

void IseIndex::Search(const std::string& queryImgPath, std::vector<std::string>& results)
{
    std::vector<Sift::Feature> sifts;
    extractor_.ExtractSift(queryImgPath, sifts, true);
    if (sifts.empty()) return;
    std::vector<unsigned> imgIds;
    switch (algo_)
    {
    case LSH:
        DoLSHSearch_(sifts, imgIds);
        break;

    case BF_SKETCH:
        DoBFSearch_(sifts, imgIds);
        break;

    case PSM:
        DoPSMSearch_(sifts, imgIds);
        break;

    case BF_SIMHASH:
        DoBFSimHashSearch_(sifts, imgIds);
        break;

    default:
        break;
    }
    std::sort(imgIds.begin(), imgIds.end());
    imgIds.resize(std::unique(imgIds.begin(), imgIds.end()) - imgIds.begin());
    for (unsigned i = 0; i < imgIds.size(); ++i)
    {
        std::string img;
        if (imgMetaStorage_.get(imgIds[i], img))
            results.push_back(img);
    }
}

void IseIndex::DoLSHSearch_(std::vector<Sift::Feature>& sifts, std::vector<unsigned>& results)
{
    for (unsigned i = 0; i < sifts.size(); ++i)
    {
        lshIndex_.Search(&sifts[i].desc[0], results);
    }
}

void IseIndex::DoBFSearch_(std::vector<Sift::Feature>& sifts, std::vector<unsigned>& results)
{
    std::vector<Sketch> sketches;
    extractor_.BuildSketch(sifts, sketches);
    bruteForce_.Search(sketches, results);
}

void IseIndex::DoPSMSearch_(std::vector<Sift::Feature>& sifts, std::vector<unsigned>& results)
{
    probSimMatch_.Search(sifts, results);
}

void IseIndex::DoBFSimHashSearch_(std::vector<Sift::Feature>& sifts, std::vector<unsigned>& results)
{
    probSimMatch_.Search(sifts, results, true);
}

void IseIndex::DoPostFiltering_(std::vector<unsigned>& in, std::vector<unsigned>& out)
{
}

}}
