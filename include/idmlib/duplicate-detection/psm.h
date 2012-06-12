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

//#define PSM_DEBUG
//#define PSM_SEARCH_DEBUG

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

template<uint32_t f, uint32_t h, uint32_t p>
class WeakBitCombination
{
    struct ItemType
    {
        ItemType() :windex(0), score(0.0), index(0)
        {
        }
        uint32_t windex;
        double score;
        uint32_t index;

        bool operator<(const ItemType& item) const
        {
            if(score>item.score) return true;
            else if(score<item.score) return false;
            return windex<item.windex;
        }

        friend std::ostream& operator<<(std::ostream& out, const ItemType& item)
        {
            out<<"windex "<<item.windex<<", score "<<item.score<<", index "<<item.index;
            return out;
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
            if(score<item.score) return true;
            else if(score>item.score) return false;
            else
            {
                return item_list.size()>item.item_list.size();
            }
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
#ifdef PSM_SEARCH_DEBUG
        //LOG(INFO)<<"sqrt_sum "<<sqrt_sum<<std::endl;
#endif
        for(uint32_t i=f-p;i<weight_vector.size();i++)
        {
            ItemType item;
            item.windex = i;
            item.score = 1.0-std::abs(weight_vector[i])/sqrt_sum;
#ifdef PSM_SEARCH_DEBUG
            //LOG(INFO)<<"add item "<<item<<std::endl;
#endif
            array_.push_back(item);
        }
        std::sort(array_.begin(), array_.end());
        for(uint32_t i=0;i<array_.size();i++)
        {
            array_[i].index = i;
#ifdef PSM_SEARCH_DEBUG
            //ItemType& item = array_[i];
            //LOG(INFO)<<"after sort item "<<item<<std::endl;
#endif
        }
        for(uint32_t i=0;i<h;i++)
        {
            std::vector<ItemType> vec(i+1);
            for(uint32_t j=0;j<i+1;j++)
            {
                vec[j] = array_[j];
            }
            SortType st(vec);
#ifdef PSM_SEARCH_DEBUG
            //LOG(INFO)<<"add item_list size "<<st.item_list.size()<<" to queue"<<std::endl;
#endif
            queue_.push(st);
        }
    }

    void Next(IntType* bits)
    {
        const SortType& st = queue_.top();

        {
#ifdef PSM_SEARCH_DEBUG
            //LOG(INFO)<<"flip count "<<st.item_list.size()<<std::endl;
#endif
            for(uint32_t i=0;i<st.item_list.size();i++)
            {
                uint32_t windex = st.item_list[i].windex;
                uint64_t v = (1ul<<(windex%64));
                bits[windex/64]+=v;
#ifdef PSM_SEARCH_DEBUG
                LOG(INFO)<<"flipping bit "<<windex<<std::endl;
#endif
            }
        }

        //generate b1, b2, and insert to heap
        SortType b1, b2;
#ifdef PSM_SEARCH_DEBUG
        //LOG(INFO)<<"next_ begin"<<std::endl;
#endif
        Next_(st, b1, b2);
#ifdef PSM_SEARCH_DEBUG
        //LOG(INFO)<<"next_ end"<<std::endl;
#endif
        queue_.pop();
        if(!b1.empty())
        {
            queue_.push(b1);
        }
        if(!b2.empty())
        {
            queue_.push(b2);
        }
#ifdef PSM_SEARCH_DEBUG
        //LOG(INFO)<<"next end"<<std::endl;
#endif

    }

private:

    void Next_(const SortType& b, SortType& b1, SortType& b2)
    {
        uint32_t l = b.item_list.size();
        uint32_t last_aindex = b.item_list.back().index+1;
        if(last_aindex<array_.size())
        {
            for(uint32_t i=0;i<l-1;i++)
            {
                b1.item_list.push_back(b.item_list[i]);
            }
            b1.item_list.push_back(array_[b.item_list.back().index+1]);
        }
        else
        {
            return;
        }
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
                            uint32_t aindex = b.item_list[i].index+1;
                            if(aindex>=array_.size())
                            {
                                b2.item_list.clear();
                                break;
                            }
                            b2.item_list.push_back(array_[aindex]);
                        }
                        else
                        {
                            b2.item_list.push_back(b.item_list[i]);
                        }
                    }
                    break;
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

template <uint32_t f, uint32_t h, uint32_t p, class KeyType, class StringType, class AttachType>
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
     , k_(21), FP_SIZE(FingerprintType::FP_SIZE)
    {
    }
    ~PSM()
    {
        if(writer_!=NULL) delete writer_;
        if(hash_!=NULL) delete hash_;
    }

    void SetK(uint32_t k) {k_ = k;}

    bool Open()
    {
        boost::filesystem::create_directories(path_);
        NewHash_();
        return Load_();
    }

    void Insert(const KeyType& key, const DocVector& doc_vector, const AttachType& attach = AttachType())
    {
        FingerprintType fp(key, attach);
        ToFingerprint(doc_vector, fp.bits);
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
        boost::filesystem::remove_all(path_+"/table.tmp");
        SortTable_();
        BuildHash_();
        return Save_();
    }

    bool Search(const DocVector& doc_vector, const AttachType& attach, KeyType& key)
    {
        WeightVector weight_vector;
        ToWeightVector(doc_vector, weight_vector);
        IntType bits[FP_SIZE];
        ToFingerprint(weight_vector, bits);
#ifdef PSM_SEARCH_DEBUG
        //LOG(INFO)<<"diff in table_[0].bits[0]  "<<GetDistance(bits[0], table_[0].bits[0])<<std::endl;
#endif
        //for(uint32_t i=0;i<table_.size();i++)
        //{
            //LOG(INFO)<<"diff in table_["<<i<<"].bits[0]  "<<GetDistance(bits[0], table_[i].bits[0])<<std::endl;
        //}
        IntType flip[FP_SIZE];
        memcpy(flip, bits, sizeof(bits));
        WeakBitCombination<f,h,p> wbc(weight_vector);
        for(uint32_t i=0;i<=k_;i++)
        {
#ifdef PSM_SEARCH_DEBUG
            LOG(INFO)<<"k index "<<i<<std::endl;
#endif
            //get flip for i>0
            if(i>0)
            {
                for(uint32_t ff=0;ff<FP_SIZE;ff++)
                {
                    flip[ff] = 0;
                }
#ifdef PSM_SEARCH_DEBUG
                //LOG(INFO)<<"before flip bits "<<BitsToString_(flip)<<std::endl;
#endif
                wbc.Next(flip);
#ifdef PSM_SEARCH_DEBUG
                //LOG(INFO)<<"flip bits "<<BitsToString_(flip)<<std::endl;
#endif
                for(uint32_t ff=0;ff<FP_SIZE;ff++)
                {
                    flip[ff] ^= bits[ff];
                }
            }
            uint64_t p_value = GetTopValue_(flip);
            TableIndex index = hash_->getInteger(p_value);
#ifdef PSM_SEARCH_DEBUG
            //LOG(INFO)<<"after bits "<<BitsToString_(flip)<<std::endl;
            //LOG(INFO)<<"pvalue "<<p_value<<std::endl;
            //LOG(INFO)<<"hash index "<<index<<std::endl;
#endif
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
                    uint32_t ffdiff = GetDistance(bits[ff], fp.bits[ff]);
                    diff += ffdiff;
                    //LOG(INFO)<<"distance on [0] "<<diff<<","<<flip[ff]<<","<<fp.bits[ff]<<std::endl;
                    if(diff>h) break;
                }
                if(diff>h) continue;
#ifdef PSM_SEARCH_DEBUG
                LOG(INFO)<<"find bits match at index "<<index<<std::endl;
#endif
                if(attach.dd(fp.attach))
                {
                    find_match = true;
                    key = fp.key;
#ifdef PSM_SEARCH_DEBUG
                    LOG(INFO)<<"find complete match at "<<index<<", key "<<key<<std::endl;
#endif
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

    void ToWeightVector(const DocVector& doc_vector, WeightVector& weight_vector) const
    {
        weight_vector.resize(f, 0.0);
        for(uint32_t i=0;i<doc_vector.size();i++)
        {
            uint64_t hash = izenelib::util::HashFunction<StringType>::generateHash64(doc_vector[i].first);
#ifdef PSM_DEBUG
            //LOG(INFO)<<"hash "<<hash<<std::endl;
#endif
            for(uint32_t ff=0;ff<f;ff++)
            {
                uint32_t fff = ff%64;
                uint64_t v = (1ul<<fff);
                v = hash&v;
                double diff = doc_vector[i].second;
                if(v==0)
                {
                    diff *= -1;
                }
#ifdef PSM_DEBUG
                //LOG(INFO)<<"position "<<ff<<", weight "<<diff<<std::endl;
#endif
                weight_vector[ff] += diff;
            }
        }
    }

    void ToFingerprint(const WeightVector& weight_vector, IntType* bits) const
    {
        uint64_t value = 0;
        uint32_t index = 0;
        for(uint32_t i=0;i<weight_vector.size();i++)
        {
            uint64_t v = 0;
            if(weight_vector[i]>=0.0) v=1;
            v <<= i;
            value+=v;
#ifdef PSM_DEBUG
            //LOG(INFO)<<"position "<<i<<" : "<<v<<","<<value<<std::endl;
#endif
            if(i%64==63)
            {
#ifdef PSM_DEBUG
                //LOG(INFO)<<"bits "<<index<<" set to "<<value<<std::endl;
#endif
                bits[index] = value;
                index+=1;
                value = 0;
            }
        }
    }

    void ToFingerprint(const DocVector& doc_vector, IntType* bits) const
    {
        WeightVector weight_vector;
        ToWeightVector(doc_vector, weight_vector);
        ToFingerprint(weight_vector, bits);
    }

    uint32_t GetDistance(uint64_t v1, uint64_t v2) const
    {
        uint32_t diff = 0;
        uint64_t db = v1^v2;
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
        return diff;
    }
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


    void SortTable_()
    {
        std::sort(table_.begin(), table_.end());
    }

    PType GetTopValue_(const IntType* bits) const
    {
        //get the top p bits value of fp
        uint64_t i = bits[FP_SIZE-1];
        i = (i >> (64-p));
        return i;
    }

    void BuildHash_()
    {
        NewHash_();
        PType last = 0;
        for(uint32_t i=0;i<table_.size();i++)
        {
#ifdef PSM_DEBUG
            std::string bits_str = BitsToString_(table_[i].bits);
            LOG(INFO)<<"table "<<i<<", bits "<<bits_str<<", top p "<<bits_str.substr(0,p_)<<std::endl;
#endif
            PType value = GetTopValue_(table_[i].bits);
#ifdef PSM_DEBUG
            LOG(INFO)<<"pvalue "<<value<<std::endl;
#endif
            if(i>0 && value==last) continue;
            hash_->setInteger(value, i);
            last = value;
        }
//      if(hash_->build()<0)
//      {
//          LOG(ERROR)<<"build hash error"<<std::endl;
//      }
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


    std::string BitsToString_(IntType* bits) const
    {
        std::string result;
        result.resize(f);
        for(uint32_t ff=0;ff<f;ff++)
        {
            std::size_t s_pos = f-ff-1;
            uint64_t value = bits[ff/64];
            uint32_t fff = ff%64;
            uint64_t v = (1ul<<fff);
            v = v&value;
            if(v>0)
            {
                result[s_pos] = '1';
            }
            else
            {
                result[s_pos] = '0';
            }
        }
        return result;
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
