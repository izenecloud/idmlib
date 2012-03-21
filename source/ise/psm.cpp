#include <idmlib/ise/psm.hpp>

#include <boost/filesystem.hpp>


namespace {

unsigned CountBits(uint64_t v)
{
    v -= (v >> 1) & 0x5555555555555555ULL;
    v = (v & 0x3333333333333333ULL) + ((v >> 2) & 0x3333333333333333ULL);
    v = (v + (v >> 4)) & 0xf0f0f0f0f0f0f0fULL;
    return (v * 0x101010101010101ULL) >> 56;
}

unsigned CalcHammingDist(const uint64_t* v1, const uint64_t* v2, unsigned count)
{
    unsigned dist = 0;
    for (unsigned i = 0; i < count; i++)
    {
        dist += CountBits(v1[i] ^ v2[i]);
    }
    return dist;
}

}

namespace idmlib{ namespace ise{

BruteForce::BruteForce(const std::string& path)
    : sketch_path_(path + "/sketches")
    , drum_path_(path + "/sketch_to_imgid")
{
}

BruteForce::~BruteForce()
{
}

void BruteForce::Init()
{
    boost::filesystem::create_directories(drum_path_);
    sketch_to_imgid_.reset(new SketchToImgIdType(drum_path_, 64, 32768, 4194304));

    std::ifstream ifs(sketch_path_.c_str(), std::ios_base::binary);
    LoadSketches_(ifs);
}

void BruteForce::LoadSketches_(std::istream &iar)
{
    if (!iar) return;
    unsigned len;
    iar.read((char *)&len, sizeof(len));
    sketches_.resize(len);
    iar.read((char *)&sketches_[0], len * sizeof(Sketch));
}

void BruteForce::Finish()
{
    std::sort(sketches_.begin(), sketches_.end());
    std::ofstream ofs(sketch_path_.c_str(), std::ios_base::binary);
    SaveSketches_(ofs);

    sketch_to_imgid_->Synchronize();
}

void BruteForce::SaveSketches_(std::ostream &oar)
{
    if (!oar) return;
    unsigned len = sketches_.size();
    oar.write((char *)&len, sizeof(len));
    oar.write((char *)&sketches_[0], len * sizeof(Sketch));
}

void BruteForce::Insert(unsigned key, const std::vector<Sketch>& sketches)
{
    sketches_.insert(sketches_.end(), sketches.begin(), sketches.end());

    std::set<unsigned> key_set;
    key_set.insert(key);
    for (unsigned i = 0; i < sketches.size(); i++)
    {
        sketch_to_imgid_->Append(sketches[i], key_set);
    }
}

void BruteForce::Delete(unsigned key)
{
    // TODO
}

void BruteForce::Search(const std::vector<Sketch>& sketches, std::vector<unsigned>& results) const
{
    for (unsigned i = 0; i < sketches.size(); i++)
    {
        for (unsigned j = 0; j < sketches_.size(); j++)
        {
            if (CalcHammingDist(sketches[i].desc, sketches_[j].desc, SKETCH_SIZE) <= 3)
            {
                std::set<unsigned> key_set;
                sketch_to_imgid_->GetValue(sketches_[j], key_set);
                results.insert(results.end(), key_set.begin(), key_set.end());
            }
        }
    }
}

ProbSimMatch::ProbSimMatch(const std::string& path)
    : simhash_path_(path + "/simhashes")
    , drum_path_(path + "/simhash_to_imgid")
{
}

ProbSimMatch::~ProbSimMatch()
{
}

void ProbSimMatch::Init()
{
    boost::filesystem::create_directories(drum_path_);
    simhash_to_imgid_.reset(new SimHashToImgIdType(drum_path_, 64, 32768, 4194304));

    simhashes_.resize(1 << (Parameter::p));
    std::ifstream ifs(simhash_path_.c_str(), std::ios_base::binary);
    LoadSimHashes_(ifs);
}

void ProbSimMatch::LoadSimHashes_(std::istream &iar)
{
    if (!iar) return;
    for (unsigned i = 0; i < simhashes_.size(); i++)
    {
        unsigned len;
        iar.read((char *)&len, sizeof(len));
        simhashes_[i].resize(len);
        iar.read((char *)&simhashes_[i][0], len * sizeof(SimHash));
    }
}

void ProbSimMatch::Finish()
{
    for (unsigned i = 0; i < simhashes_.size(); i++)
    {
        std::sort(simhashes_[i].begin(), simhashes_[i].end());
    }
    std::ofstream ofs(simhash_path_.c_str(), std::ios_base::binary);
    SaveSimHashes_(ofs);

    simhash_to_imgid_->Synchronize();
}

void ProbSimMatch::SaveSimHashes_(std::ostream &oar)
{
    if (!oar) return;
    for (unsigned i = 0; i < simhashes_.size(); i++)
    {
        unsigned len = simhashes_[i].size();
        oar.write((char *)&len, sizeof(len));
        oar.write((char *)&simhashes_[i][0], len * sizeof(SimHash));
    }
}

void ProbSimMatch::Insert(unsigned key, const std::vector<Sift::Feature>& sifts)
{
    std::set<unsigned> key_set;
    key_set.insert(key);

    for (unsigned i = 0; i < sifts.size(); i++)
    {
        SimHash simhash;
        GenSimHash_(sifts[i].desc, simhash);
        unsigned index = simhash.desc[0] >> (CHUNK_BIT - Parameter::p);
        simhashes_[index].push_back(simhash);

        simhash_to_imgid_->Append(simhash, key_set);
    }
}

void ProbSimMatch::GenSimHash_(const std::vector<float>& components, SimHash& simhash) const
{
    // TODO
}

void ProbSimMatch::Delete(unsigned key)
{
    // TODO
}

void ProbSimMatch::Search(const std::vector<Sift::Feature>& sifts, std::vector<unsigned>& results) const
{
    for (unsigned i = 0; i < sifts.size(); i++)
    {
        SimHash simhash;
        std::vector<unsigned> table_ids;
        GenTableIds_(sifts[i].desc, simhash, table_ids);
        for (std::vector<unsigned>::const_iterator it = table_ids.begin();
                it != table_ids.end(); ++it)
        {
            for (unsigned j = 0; j < simhashes_[*it].size(); j++)
            {
                if (CalcHammingDist(simhash.desc, simhashes_[*it][j].desc, SIMHASH_SIZE) <= 3)
                {
                    std::set<unsigned> key_set;
                    simhash_to_imgid_->GetValue(simhashes_[*it][j], key_set);
                    results.insert(results.end(), key_set.begin(), key_set.end());
                }
            }
        }
    }
}

void ProbSimMatch::GenTableIds_(const std::vector<float>& components, SimHash& simhash, std::vector<unsigned>& table_ids) const
{
    // TODO
}

}}
