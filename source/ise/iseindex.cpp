#include <idmlib/ise/iseindex.hpp>

#include <boost/filesystem.hpp>

namespace bfs = boost::filesystem;

namespace idmlib{ namespace ise{

IseIndex::IseIndex(const std::string& homePath)
    :lshHome_(bfs::path(bfs::path(homePath)/"lshindex").string())
    ,sketchHome_(bfs::path(bfs::path(homePath)/"sketchindex").string())
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

    if(bfs::exists(sketchHome_))
    {
        LoadSketch_();
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
    SaveLSH_();
    SaveSketch_();
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

void IseIndex::LoadSketch_()
{
    std::ifstream ar(sketchHome_.c_str(), std::ios_base::binary);
    unsigned L;
    ar & L;
    sketches_.resize(L);
    for (unsigned i = 0; i < L; ++i) {
        Sketch sketch;
        ar.read((char *)&sketch, sizeof(Sketch));
        unsigned id;
        ar.read((char *)&id, sizeof(unsigned));
        sketches_[i] = std::make_pair(sketch,id);
    }
}

void IseIndex::SaveSketch_()
{
    std::ofstream ar(sketchHome_.c_str(), std::ios_base::binary);
    unsigned L;
    L = sketches_.size();
    ar & L;
    for (unsigned i = 0; i < L; ++i) {
        ar.write((char *)&sketches_[i].first, sizeof(Sketch));
        ar.write((char *)&sketches_[i].second, sizeof(unsigned));
    }
}

bool IseIndex::Insert(const std::string& imgPath)
{
    static unsigned id = 0;
    ++id;
    std::vector<Sift::Feature> sift;
    extractor_.ExtractSift(imgPath, sift, false);
    if(sift.empty()) return false;
    std::vector<Sketch > sketches;
    extractor_.BuildSketch(sift, sketches);
    for(unsigned i = 0; i < sketches.size(); ++i)
        sketches_.push_back(std::make_pair(sketches[i],id));
    for(unsigned i = 0; i < sift.size(); ++i)
        lshIndex_.Insert(&sift[i].desc[0],id);
    
    imgMetaStorage_.insert(id,imgPath);
    return true;
}

void IseIndex::Search(const std::string& queryImgPath, std::vector<std::string>& results)
{
    std::vector<Sift::Feature> sift;
    extractor_.ExtractSift(queryImgPath, sift, true);
    std::vector<unsigned> imgIds;
    //DoLSHSearch_(sift, imgIds);
    DoSketchSearch_(sift, imgIds);
    for(unsigned i = 0; i < imgIds.size(); ++i)
    {
        std::string img;
        if(imgMetaStorage_.get(imgIds[i],img))
            results.push_back(img);
    }
}

void IseIndex::DoLSHSearch_(std::vector<Sift::Feature>& sift, std::vector<unsigned>& results)
{
    for(unsigned i = 0; i < sift.size(); ++i)
    {
        lshIndex_.Search(&sift[i].desc[0], results);
    }
    std::sort(results.begin(), results.end());		
    results.resize(std::unique(results.begin(), results.end()) - results.begin());		
}

void IseIndex::DoSketchSearch_(std::vector<Sift::Feature>& sift, std::vector<unsigned>& results)
{
    std::vector<Sketch > sketches;
    extractor_.BuildSketch(sift, sketches);
    Hamming hamming;
    for(unsigned i = 0; i < sketches.size(); ++i)
    {
        for(unsigned j = 0; j < sketches_.size(); ++j)
        {
            if(hamming(sketches[i].desc,sketches_[j].first.desc) < 4)
            {
                results.push_back(sketches_[j].second);
            }
        }
    }
    std::sort(results.begin(), results.end());		
    results.resize(std::unique(results.begin(), results.end()) - results.begin());		
}

void IseIndex::DoPostFiltering_(std::vector<unsigned>& in, std::vector<unsigned>& out)
{

}

}}
