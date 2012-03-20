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

ProbSimMatch::ProbSimMatch(const std::string& path)
    : sketch_path_(path + "/sketches")
    , drum_path_(path + "/sketch_to_imgid")
{
}

ProbSimMatch::~ProbSimMatch()
{
}

void ProbSimMatch::Init()
{
    boost::filesystem::create_directories(drum_path_);
    sketch_to_imgid_.reset(new SketchToImgIdType(drum_path_, 64, 32768, 4194304));

    sketches_.resize(1 << (Parameter::p));
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
    for (unsigned i = 0; i < sketches.size(); i++)
    {
        unsigned index = sketches[i].desc[0] >> (sizeof(key) * 8 - Parameter::p);
        sketches_[index].push_back(sketches[i]);

        std::set<key_t> key_set;
        key_set.insert(key);
        sketch_to_imgid_->Append(sketches[i], key_set);
    }
}

void ProbSimMatch::Delete(key_t key)
{
}

void ProbSimMatch::BFSearch(const std::vector<Sketch>& sketches, std::vector<key_t>& results) const
{
    for (unsigned i = 0; i < sketches.size(); i++)
    {
        for (unsigned j = 0; j < sketches_.size(); j++)
        {
            for (unsigned k = 0; k < sketches_[j].size(); k++)
            {
                if (CalcHammingDist_(sketches[i], sketches_[j][k]) <= 3)
                {
                    std::set<key_t> key_set;
                    sketch_to_imgid_->GetValue(sketches_[j][k], key_set);
                    results.insert(results.end(), key_set.begin(), key_set.end());
                }
            }
        }
    }
}

void ProbSimMatch::PSMSearch(
        const std::vector<Sift::Feature>& sifts,
        const std::vector<Sketch>& sketches,
        std::vector<key_t>& results) const
{
    for (unsigned i = 0; i < sifts.size(); i++)
    {
        std::vector<unsigned> table_ids;
        GenTableIds_(sifts[i].desc, table_ids);
        for (std::vector<unsigned>::const_iterator it = table_ids.begin();
                it != table_ids.end(); ++it)
        {
            for (unsigned k = 0; k < sketches_[*it].size(); k++)
            {
                if (CalcHammingDist_(sketches[i], sketches_[*it][k]) <= 3)
                {
                    std::set<key_t> key_set;
                    sketch_to_imgid_->GetValue(sketches_[*it][k], key_set);
                    results.insert(results.end(), key_set.begin(), key_set.end());
                }
            }
        }
    }
}

void ProbSimMatch::GenTableIds_(const std::vector<float>& components, std::vector<unsigned>& table_ids) const
{
    // TODO
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
