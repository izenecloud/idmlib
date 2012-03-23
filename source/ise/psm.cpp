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

bool AbsCompare(const std::vector<float>& vec, unsigned index1, unsigned index2)
{
    return fabsf(vec[index1]) < fabsf(vec[index2]);
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
    sketch_to_imgid_.reset(new SketchToImgIdType(drum_path_, 64, 131072, 4194304));

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
    , rand_vec_path_(path + "/rand_vec_table")
{
}

ProbSimMatch::~ProbSimMatch()
{
}

void ProbSimMatch::Init()
{
    boost::filesystem::create_directories(drum_path_);
    simhash_to_imgid_.reset(new SimHashToImgIdType(drum_path_, 64, 131072, 4194304));

    simhashes_.resize(1 << (Parameter::p));
    std::ifstream ifs(simhash_path_.c_str(), std::ios_base::binary);
    LoadSimHashes_(ifs);

    InitRandVecTable_();

//  bit_flip_table_.reserve(Parameter::k);
//  for (unsigned i = 0; i < Parameter::ki; i++)
//  {
//      bit_flip_table_.push_back(1 << i);
//  }
//  unsigned index = 0;
//  for (unsigned i = 0; i < Parameter::ki - 1; i++)
//  {
//      unsigned id1 = bit_flip_table_[index++];
//      for (unsigned j = i + 1; j < Parameter::ki; j++)
//      {
//          bit_flip_table_.push_back(id1 | (1 << j));
//      }
//  }
    bit_flip_table_.resize(Parameter::k);
    std::vector<std::vector<unsigned> >::iterator it = bit_flip_table_.begin();
    for (unsigned i = 0; i < Parameter::ki; i++)
    {
        it->resize(1, i);
        ++it;
    }
    for (unsigned i = 0; i < Parameter::ki - 1; i++)
    {
        for (unsigned j = i + 1; j < Parameter::ki; j++)
        {
            it->reserve(2);
            it->push_back(i);
            it->push_back(j);
            ++it;
        }
    }
}

void ProbSimMatch::InitRandVecTable_()
{
    rand_vec_table_.resize(Sift::dim());
    std::ifstream ifs(rand_vec_path_.c_str(), std::ios_base::binary);
    if (ifs)
    {
        ifs.read((char *)&rand_vec_table_[0], Sift::dim() * sizeof(SimHash));
    }
    else
    {
        lshkit::DefaultRng rng;
        rng.seed(std::time(0));
        for (unsigned i = 0; i < Sift::dim(); i++)
        {
            for (unsigned j = 0; j < SIMHASH_SIZE; j++)
            {
                rand_vec_table_[i].desc[j] = uint64_t(rng()) | (uint64_t(rng()) << 32);
            }
        }
        std::ofstream ofs(rand_vec_path_.c_str(), std::ios_base::binary);
        ofs.write((char *)&rand_vec_table_[0], Sift::dim() * sizeof(SimHash));
    }
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
    std::vector<float> char_vec;
    SimHash simhash;
    std::set<unsigned> key_set;
    key_set.insert(key);

    unsigned bit_mask = (1 << Parameter::p) - 1;
    for (unsigned i = 0; i < sifts.size(); i++)
    {
        MapFeatureToCharVec_(sifts[i].desc, char_vec);
        GenSimHash_(char_vec, simhash);
        unsigned id = simhash.desc[0] & bit_mask;
        simhashes_[id].push_back(simhash);

        simhash_to_imgid_->Append(simhash, key_set);
    }
}

void ProbSimMatch::MapFeatureToCharVec_(const std::vector<float>& feature, std::vector<float>& char_vec) const
{
    char_vec.clear();
    char_vec.resize(SIMHASH_BIT);
    for (unsigned i = 0; i < Sift::dim(); i++)
    {
        std::vector<float>::iterator it = char_vec.begin();
        for (unsigned j = 0; j < SIMHASH_SIZE; j++)
        {
            Chunk chunk = rand_vec_table_[i].desc[j];
            for (unsigned k = 0; k < CHUNK_BIT; k++)
            {
                if ((chunk >> k) & 0x1)
                    *it += feature[i];
                else
                    *it -= feature[i];
                ++it;
            }
        }
    }
}

void ProbSimMatch::GenSimHash_(const std::vector<float>& char_vec, SimHash& simhash) const
{
    std::vector<float>::const_iterator it = char_vec.begin();
    for (unsigned i = 0; i < SIMHASH_SIZE; i++)
    {
        simhash.desc[i] = 0;
        for (unsigned j = 0; j < CHUNK_BIT; j++)
        {
            if (*it >= 0.)
                simhash.desc[i] |= (Chunk)1 << j;
            ++it;
        }
    }
}

void ProbSimMatch::Delete(unsigned key)
{
    // TODO
}

void ProbSimMatch::Search(const std::vector<Sift::Feature>& sifts, std::vector<unsigned>& results, bool brute_force) const
{
    std::vector<float> char_vec;
    SimHash simhash;
    for (unsigned i = 0; i < sifts.size(); i++)
    {
        MapFeatureToCharVec_(sifts[i].desc, char_vec);
        if (brute_force)
        {
            GenSimHash_(char_vec, simhash);
            for (unsigned j = 0; j < simhashes_.size(); j++)
            {
                const std::vector<SimHash>& table = simhashes_[j];
                for (unsigned k = 0; k < table.size(); k++)
                {
                    if (CalcHammingDist(simhash.desc, table[k].desc, SIMHASH_SIZE) <= Parameter::h)
                    {
                        std::set<unsigned> key_set;
                        simhash_to_imgid_->GetValue(table[k], key_set);
                        results.insert(results.end(), key_set.begin(), key_set.end());
                    }
                }
            }
        }
        else
        {
            std::vector<unsigned> table_ids;
            GenTableIds_(char_vec, simhash, table_ids);
            for (unsigned j = 0; j < table_ids.size(); j++)
            {
                const std::vector<SimHash>& table = simhashes_[table_ids[j]];
                for (unsigned k = 0; k < table.size(); k++)
                {
                    if (CalcHammingDist(simhash.desc, table[k].desc, SIMHASH_SIZE) <= Parameter::h)
                    {
                        std::set<unsigned> key_set;
                        simhash_to_imgid_->GetValue(table[k], key_set);
                        results.insert(results.end(), key_set.begin(), key_set.end());
                    }
                }
            }
        }
    }
}

void ProbSimMatch::GenTableIds_(const std::vector<float>& char_vec, SimHash& simhash, std::vector<unsigned>& table_ids) const
{
    GenSimHash_(char_vec, simhash);

    std::vector<unsigned> prob_order(Parameter::p);
    for (unsigned i = 1; i < Parameter::p; i++)
    {
        prob_order[i] = i;
    }
    std::sort(prob_order.begin(), prob_order.end(), boost::bind(AbsCompare, char_vec, _1, _2));

    table_ids.clear();
    table_ids.reserve(bit_flip_table_.size() + 1);
    unsigned id = simhash.desc[0] & ((1 << Parameter::p) - 1);
    table_ids.push_back(id);
//  for (unsigned i = 0; i < bit_flip_table_.size(); i++)
//  {
//      table_ids.push_back(id ^ bit_flip_table_[i]);
//  }
    for (unsigned i = 0; i < bit_flip_table_.size(); i++)
    {
        unsigned new_id = id;
        for (std::vector<unsigned>::const_iterator it = bit_flip_table_[i].begin();
                it != bit_flip_table_[i].end(); ++it)
        {
            new_id ^= 1 << prob_order[*it];
        }
        table_ids.push_back(new_id);
    }
}

}}
