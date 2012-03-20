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

}

namespace idmlib{ namespace ise{

ProbSimMatch::ProbSimMatch(const std::string& path, bool brute_force)
    : sketch_path_(path + "/sketches")
    , drum_path_(path + "/sketch_to_imgid")
    , brute_force_(brute_force)
{
}

ProbSimMatch::~ProbSimMatch()
{
}

void ProbSimMatch::Init()
{
    sketches_.resize(1 << (Parameter::p));
    boost::filesystem::create_directories(drum_path_);
    sketch_to_imgid_.reset(new SketchToImgIdType(drum_path_, 64, 32768, 4194304));
    std::ifstream ifs(sketch_path_.c_str(), std::ios_base::binary);
    LoadSketches_(ifs);
}

void ProbSimMatch::LoadSketches_(std::istream &iar)
{
    if (!iar) return;
    for (unsigned i = 0; i < sketches_.size(); i++)
    {
        unsigned len;
        iar.read((char *)&len, sizeof(len));
        sketches_[i].resize(len);
        iar.read((char *)&sketches_[i][0], len * sizeof(Sketch));
    }
}

void ProbSimMatch::SaveSketches_(std::ostream &oar)
{
    if (!oar) return;
    for (unsigned i = 0; i < sketches_.size(); i++)
    {
        unsigned len = sketches_[i].size();
        oar.write((char *)&len, sizeof(len));
        oar.write((char *)&sketches_[i][0], len * sizeof(Sketch));
    }
}

void ProbSimMatch::Insert(key_t key, const std::vector<Sketch>& sketches)
{
    for (std::vector<Sketch>::const_iterator it = sketches.begin();
            it != sketches.end(); ++it)
    {
        unsigned index = it->desc[0] >> (sizeof(key) * 8 - Parameter::p);
        sketches_[index].push_back(*it);

        std::set<key_t> key_set;
        key_set.insert(key);
        sketch_to_imgid_->Append(*it, key_set);
    }
}

void ProbSimMatch::Delete(key_t key)
{
}

void ProbSimMatch::Search(const Sketch& sketch, std::vector<key_t>& result) const
{
    std::vector<Sketch> matched;
    if (brute_force_)
    {
        for (unsigned i = 0; i < sketches_.size(); i++)
            for (unsigned j = 0; j < sketches_[i].size(); j++)
                if (CalcHammingDist_(sketch, sketches_[i][j]) <= 3)
                    matched.push_back(sketches_[i][j]);
    }
    else
    {
        // TODO
    }
    for (unsigned i = 0; i < matched.size(); i++)
    {
        std::set<key_t> key_set;
        sketch_to_imgid_->GetValue(matched[i], key_set);
        result.insert(result.end(), key_set.begin(), key_set.end());
    }
}

void ProbSimMatch::Finish()
{
    for (unsigned i = 0; i < sketches_.size(); i++)
    {
        std::sort(sketches_[i].begin(), sketches_[i].end());
    }
    std::ofstream ofs(sketch_path_.c_str(), std::ios_base::binary);
    SaveSketches_(ofs);
    sketch_to_imgid_->Synchronize();
}

unsigned ProbSimMatch::CalcHammingDist_(const Sketch& s1, const Sketch& s2) const
{
    unsigned dist = 0;
    for (unsigned i = 0; i < SKETCH_SIZE; i++)
    {
        dist += CountBits(s1.desc[i] ^ s2.desc[i]);
    }
    return dist;
}

}}
