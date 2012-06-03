#ifndef IDMLIB_DD_PSM_H_
#define IDMLIB_DD_PSM_H_

#include <algorithm>
#include <am/succinct/fujimap/fujimap.hpp>
#include <am/sequence_file/ssfr.h>
#include "dd_types.h"
#include <boost/serialization/access.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/filesystem.hpp>
#include <util/hashFunction.h>

NS_IDMLIB_DD_BEGIN

template <uint32_t f, class KeyType, class AttachType>
class Fingerprint
{
public:
    typedef Fingerprint<f, KeyType, AttachType> ThisType;
    typedef uint64_t IntType;
    static const uint32_t FP_SIZE = f / 64;
    //typedef IntType[1] BitsType;

    KeyType key;
    IntType bits[FP_SIZE];
    AttachType attach;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        for (uint32_t i = 0; i < FP_SIZE; i++) ar & bits[i];
        ar & key & attach;
    }

    Fingerprint()
    {
    }

    Fingerprint(const KeyType& p1, const AttachType& p2)
    :key(p1), attach(p2)
    {
    }

    int compare(const ThisType& other) const
    {
        for (uint32_t i = 0; i < FP_SIZE; i++)
        {
            if (bits[i] < other.bits[i])
                return -1;
            else if (bits[i] > other.bits[i])
                return 1;
        }
        if(key<other.key) return -1;
        else if(key>other.key) return 1;
        return 0;
    }

    bool operator<(const ThisType& other) const
    {
        return compare(other) == -1;
    }

};

template<uint32_t f, uint32_t h>
class WeakBitCombination
{
    struct ItemType
    {
        uint32_t windex;
        double score;
        uint32_t index;

        bool operator<(const ItemType& item) const
        {
            if(score>item.score) return true;
            else if(score<item.score) return false;
            return windex<item.windex;
        }
    };
    struct SortType
    {
        std::vector<ItemType> item_list;
        double score;
        SortType(): score(0.0)
        {
        }
        SortType(const std::vector<ItemType>& p1)
        :item_list(p1)
        {
            ComputeScore();
        }

        bool operator<(const SortType& item) const
        {
            return score>item.score;
        }

        bool empty() const
        {
            return item_list.empty();
        }
        void ComputeScore()
        {
            if(item_list.empty())
            {
                score = 0.0;
            }
            else
            {
                score = 1.0;
                for(uint32_t i=0;i<item_list.size();i++)
                {
                    score *= item_list[i].score;
                }
            }
        }
    };
public:
    typedef uint64_t IntType;
    static const uint32_t FP_SIZE = f / 64;
    WeakBitCombination(const std::vector<double>& weight_vector)
    {
        double sqrt_sum = 0.0;
        for(uint32_t i=0;i<weight_vector.size();i++)
        {
            sqrt_sum += weight_vector[i]*weight_vector[i];
        }
        sqrt_sum = std::sqrt(sqrt_sum);
        for(uint32_t i=0;i<weight_vector.size();i++)
        {
            ItemType item;
            item.windex = i;
            item.score = 1.0-std::abs(weight_vector[i])/sqrt_sum;
            array_.push_back(item);
        }
        std::sort(array_.begin(), array_.end());
        for(uint32_t i=0;i<array_.size();i++)
        {
            array_[i].index = i;
        }
        for(uint32_t i=0;i<h;i++)
        {
            std::vector<ItemType> vec(i+1);
            for(uint32_t j=0;j<i+1;j++)
            {
                vec[j] = array_[j];
            }
            SortType st(vec);
            queue_.push(st);
        }
    }

    void Next(IntType* bits)
    {
        const SortType& st = queue_.top();

        {
            for(uint32_t i=0;i<st.item_list.size();i++)
            {
                uint64_t v = 0;
                v <<= (st.item_list[i].windex%64);
                bits[st.item_list[i].windex/64]+=v;
            }
        }

        //generate b1, b2, and insert to heap
        SortType b1, b2;
        Next_(st, b1, b2);
        queue_.pop();
        if(!b1.empty())
        {
            queue_.push(b1);
        }
        if(!b2.empty())
        {
            queue_.push(b2);
        }

    }

private:

    void Next_(const SortType& b, SortType& b1, SortType& b2)
    {
        uint32_t l = b.item_list.size();
        for(uint32_t i=0;i<l-1;i++)
        {
            b1.item_list.push_back(b.item_list[i]);
        }
        b1.item_list.push_back(array_[b.item_list.back().index+1]);
        uint32_t j=1;
        while(j<l)
        {
            uint32_t diff = b.item_list[l-j].index-b.item_list[l-j-1].index;
            if(diff<=2)
            {
                if(diff==2)
                {
                    for(uint32_t i=0;i<l;i++)
                    {
                        if(i==l-j-1)
                        {
                            b2.item_list.push_back(array_[b.item_list[i].index+1]);
                        }
                        else
                        {
                            b2.item_list.push_back(b.item_list[i]);
                        }
                    }

                }
                else
                {
                    j+=1;
                }
            }
            else
            {
                break;
            }
        }
        b1.ComputeScore();
        b2.ComputeScore();
    }

private:
    std::vector<ItemType> array_;
    std::priority_queue<SortType> queue_;
};

template <uint32_t f, uint32_t h, class KeyType, class StringType, class AttachType>
class PSM
{
public:
    typedef Fingerprint<f,KeyType,AttachType> FingerprintType;
    //typedef FingerprintType::BitsType BitsType;
    typedef uint64_t IntType;
    typedef uint32_t TableIndex;
    typedef uint64_t PType;
    typedef izenelib::am::succinct::fujimap::Fujimap<PType, TableIndex> HashType;
    typedef std::vector<std::pair<StringType, double> > DocVector;
    typedef std::vector<double> WeightVector;
    typedef izenelib::am::ssf::Writer<> WriterType;
    typedef izenelib::am::ssf::Reader<> ReaderType;

    PSM(const std::string& path)
    :path_(path), writer_(NULL), hash_(NULL)
     ,p_(24), k_(21), FP_SIZE(FingerprintType::FP_SIZE)
    {
    }
    ~PSM()
    {
        if(writer_!=NULL) delete writer_;
        if(hash_!=NULL) delete hash_;
    }

    void SetK(uint32_t k) {k_ = k;}
    void SetP(uint32_t p) {p_ = p;}

    bool Open()
    {
        NewHash_();
        return Load_();
    }

    void Insert(const KeyType& key, const DocVector& doc_vector, const AttachType& attach = AttachType())
    {
        FingerprintType fp(key, attach);
        ToFingerprint_(doc_vector, fp.bits);
        WriterType* writer = GetWriter_();
        writer->Append(fp);

    }

    bool Build()
    {
        if(writer_ == NULL) return true;
        writer_->Close();
        delete writer_;
        writer_ = NULL;
        ReaderType* reader = GetReader_();
        table_.reserve(table_.size()+reader->Count());
        FingerprintType fp;
        while(reader->Next(fp))
        {
            table_.push_back(fp);
        }
        reader->Close();
        delete reader;
        SortTable_();
        BuildHash_();
        return Save_();
    }

    bool Search(const DocVector& doc_vector, const AttachType& attach, KeyType& key)
    {
        WeightVector weight_vector;
        ToWeightVector_(doc_vector, weight_vector);
        IntType bits[FP_SIZE];
        ToFingerprint_(weight_vector, bits);
        IntType flip[FP_SIZE];
        memcpy(flip, bits, sizeof(bits));
        WeakBitCombination<f,h> wbc(weight_vector);
        for(uint32_t i=0;i<=k_;i++)
        {
            //get flip for i>0
            if(i>0)
            {
                for(uint32_t ff=0;ff<FP_SIZE;ff++)
                {
                    flip[ff] = 0;
                }
                wbc.Next(flip);
                for(uint32_t ff=0;ff<FP_SIZE;ff++)
                {
                    flip[ff] ^= bits[ff];
                }
            }
            uint64_t p_value = GetTopValue_(flip);
            TableIndex index = hash_->getInteger(p_value);
            if(index == (TableIndex)izenelib::am::succinct::fujimap::NOTFOUND) continue;
            bool find_match = false;
            for(; index<table_.size();index++)
            {
                const FingerprintType& fp = table_[index];
                uint64_t fp_p_value = GetTopValue_(fp.bits);
                if(fp_p_value!=p_value) break;
                uint32_t diff = 0;
                for(uint32_t ff=0;ff<FP_SIZE;ff++)
                {
                    uint64_t db = flip[ff]^fp.bits[ff];
                    uint64_t di = 0;
                    for(uint32_t id=0;id<64;id++)
                    {
                        if(di==0) di=1;
                        else
                        {
                            di <<= 1;
                        }
                        uint64_t idb = db&di;
                        if(idb>0) diff++;
                    }
                    if(diff>h) break;
                }
                if(diff>h) continue;
                if(attach.dd(fp.attach))
                {
                    find_match = true;
                    key = fp.key;
                    break;
                }
            }
            if(find_match)
            {
                return true;
            }
        }
        return false;
    }

    //bool Search(const DocVector& doc_vector, KeyType& key)
    //{
    //}

private:

    void NewHash_()
    {
        if(hash_!=NULL)
        {
            delete hash_;
        }
        std::string hash_tmp_file = path_+"/hash.tmp";
        if(boost::filesystem::exists(hash_tmp_file))
        {
            boost::filesystem::remove_all(hash_tmp_file);
        }
        hash_ = new HashType(hash_tmp_file.c_str());
        hash_->initFP(32);
        hash_->initTmpN(30000000);
    }

    void ToWeightVector_(const DocVector& doc_vector, WeightVector& weight_vector)
    {
        weight_vector.resize(f, 0.0);
        for(uint32_t i=0;i<doc_vector.size();i++)
        {
            uint64_t hash = izenelib::util::HashFunction<StringType>::generateHash64(doc_vector[i].first);
            for(uint32_t ff=0;ff<f;ff++)
            {
                uint32_t fff = ff%64;
                uint64_t v = (1<<fff);
                v = hash&v;
                double diff = doc_vector[i].second;
                if(v==0)
                {
                    diff *= -1;
                }
                weight_vector[ff] += diff;
            }
        }
    }

    void ToFingerprint_(const WeightVector& weight_vector, IntType* bits)
    {
        uint64_t value = 0;
        uint32_t index = 0;
        for(uint32_t i=0;i<weight_vector.size();i++)
        {
            uint64_t v = 0;
            if(weight_vector[i]>=0.0) v=1;
            v <<= i;
            value+=v;
            if(i%64==63)
            {
                bits[index] = value;
                index+=1;
                value = 0;
            }
        }
    }

    void ToFingerprint_(const DocVector& doc_vector, IntType* bits)
    {
        WeightVector weight_vector;
        ToWeightVector_(doc_vector, weight_vector);
        ToFingerprint_(weight_vector, bits);
    }

    void SortTable_()
    {
        std::sort(table_.begin(), table_.end());
    }

    PType GetTopValue_(const IntType* bits) const
    {
        //get the top p bits value of fp
        uint64_t i = bits[FP_SIZE-1];
        i = (i >> (64-p_));
        return i;
    }

    void BuildHash_()
    {
        NewHash_();
        PType last = 0;
        for(uint32_t i=0;i<table_.size();i++)
        {
            PType value = GetTopValue_(table_[i].bits);
            if(i>0 && value==last) continue;
            hash_->setInteger(value, i);
            last = value;
        }
        if(hash_->build()<0)
        {
            LOG(ERROR)<<"build hash error"<<std::endl;
        }
    }

    bool Load_()
    {
        std::string table_file = path_+"/table";
        izenelib::am::ssf::Util<>::Load(table_file, table_);
        std::string hash_file = path_+"/hash";
        if(boost::filesystem::exists(hash_file))
        {
            hash_->load(hash_file.c_str());
        }
        return true;

    }

    bool Save_()
    {
        std::string table_file = path_+"/table";
        izenelib::am::ssf::Util<>::Save(table_file, table_);
        std::string hash_file = path_+"/hash";
        hash_->save(hash_file.c_str());
        return true;
    }

    WriterType* GetWriter_()
    {
        if(writer_==NULL)
        {
            std::string table_tmp = path_+"/table.tmp";
            boost::filesystem::remove_all(table_tmp);
            writer_ = new WriterType(table_tmp);
            writer_->Open();
        }
        return writer_;
    }

    ReaderType* GetReader_()
    {
        std::string table_tmp = path_+"/table.tmp";
        ReaderType* reader = new ReaderType(table_tmp);
        reader->Open();
        return reader;
    }

private:
    std::string path_;
    WriterType* writer_;
    std::vector<FingerprintType> table_;
    HashType* hash_;
    uint32_t p_;
    uint32_t k_;
    uint32_t FP_SIZE;
};



NS_IDMLIB_DD_END

#endif 

