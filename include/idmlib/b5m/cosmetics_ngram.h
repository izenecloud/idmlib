#ifndef IDMLIB_B5M_COSMETICSNGRAM_H_
#define IDMLIB_B5M_COSMETICSNGRAM_H_
#include "b5m_helper.h"
#include "b5m_types.h"
#include "scd_doc_processor.h"
#include <am/sequence_file/ssfr.h>
#include <sf1common/ScdWriter.h>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <util/izene_serialization.h>

NS_IDMLIB_B5M_BEGIN

using izenelib::util::UString;

class CosmeticsNgram {

public:
    typedef uint32_t term_t;
    typedef double score_t;
    typedef std::vector<term_t> word_t;
    typedef izenelib::am::ssf::Reader<std::size_t> Reader;
    typedef izenelib::am::ssf::Writer<std::size_t> Writer;
    typedef izenelib::am::ssf::Sorter<std::size_t, uint32_t, true> Sorter;
    typedef std::pair<term_t, std::size_t> TermCount;
    typedef std::pair<word_t, std::size_t> WordCount;
    typedef std::pair<term_t, score_t> TermScore;
    typedef std::vector<TermCount> TermCountArray;
    typedef std::vector<WordCount> WordCountArray;
    typedef std::vector<TermScore> TermScoreArray;
    typedef TermCountArray Context;
    typedef std::pair<std::string, double> StringDouble;
    typedef std::vector<StringDouble> StringDoubleArray;
    typedef boost::unordered_map<term_t, std::size_t> TermCountMap;
    typedef boost::unordered_map<term_t, score_t> CategoryScore;
    typedef boost::unordered_map<term_t, std::size_t> CategoryCount;
    typedef std::pair<term_t, term_t> cbid_t;
    struct NCI
    {
        NCI() :cid(0), bid(0), freq(0)
        {
        }
        term_t cid;
        term_t bid;
        std::size_t freq;
        StringDoubleArray cpa;//capacity price array;
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version)
        {
            ar & cid & bid & freq & cpa;
        }
        bool operator<(const NCI& y) const
        {
            if(cid!=y.cid) return cid<y.cid;
            return bid<y.bid;
        }
        bool IdEqual(const NCI& y) const
        {
            return cid==y.cid&&bid==y.bid;
        }
    };
    typedef std::vector<NCI> NCIArray;
    typedef boost::unordered_map<cbid_t, NCI> CategoryNCI;
    struct Ngram
    {
        word_t word;
        std::vector<std::string> text;
        term_t left_term;//0 if empty
        term_t right_term;//0 if empty
        term_t oid;
        term_t cid;
        term_t bid;//brand id
        double price;
        std::string capacity;
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version)
        {
            ar & word & text & left_term & right_term & oid & cid & bid & price & capacity;
        }
        Ngram(): left_term(0), right_term(0), oid(0), cid(0), bid(0), price(0.0)
        {
        }
        bool operator<(const Ngram& y) const
        {
            if(word<y.word) return true;
            else if(word>y.word) return false;
            else return oid<y.oid;
        }
        std::string GetText() const
        {
            std::stringstream ss;
            for(std::size_t i=0;i<text.size();i++)
            {
                ss<<text[i];
            }
            return ss.str();
        }

        static const uint32_t GetContextLength()
        {
            static const uint32_t l = 1;
            return l;
        }
        bool Pick(uint32_t index, Ngram& ngram) const
        {
            if(index>=word.size()) return false;
            ngram = *this;
            ngram.word.clear();
            ngram.text.clear();
            ngram.word.assign(word.begin()+index, word.end());
            ngram.text.assign(text.begin()+index, text.end());
            if(index>0)
            {
                ngram.left_term = word[index-1];
            }
            return true;
        }
        bool Serialize(uint32_t index, word_t& data) const
        {
            if(index>=word.size()) return false;
#ifdef NGRAMP_DEBUG
            std::cerr<<"[W]";
            for(std::size_t i=index;i<word.size();i++)
            {
                std::cerr<<word[i]<<",";
            }
            std::cerr<<std::endl;
            std::cerr<<"[INDEX]"<<index<<std::endl;
#endif
            Ngram ser(*this);
            ser.word.clear();
            ser.text.clear();
            ser.text.assign(text.begin()+index, text.end());
            if(index>0)
            {
                ser.left_term = word[index-1];
            }
            
            char* ptr;
            std::size_t len;
            Serializer::type izs(ser);
            izs.write_image(ptr, len);
            std::size_t prop = sizeof(term_t);
            std::size_t plen = (word.size()-index+2+2)*sizeof(term_t)+len;
            term_t extra = len%prop;//number of 0 in the end
            if(extra!=0)
            {
                extra = prop-extra;
                plen += extra;
            }
            term_t zero = 0;
            char* pdata = new char[plen];
            char* opdata = pdata;
            std::size_t step = (word.size()-index)*prop;
            memcpy(pdata, &word[index], step);
            pdata+=step;
            step = prop;
            memcpy(pdata, &oid, step);
            pdata+=step;
            //split zero
            step = prop;
            memcpy(pdata, &zero, step);
            pdata+=step;
            step = sizeof(std::size_t);
            memcpy(pdata, &len, step);
            pdata+=step;
            memcpy(pdata, ptr, len);
            pdata+=len;
            if(extra>0)
            {
                memcpy(pdata, &zero, extra);
            }
            char* opend = opdata+plen;
            data.assign((term_t*)opdata, (term_t*)opend);
            delete[] opdata;
            return true;
        }
        void Deserialize(const word_t& data)
        {
            std::size_t p=0;
            for(;p<data.size();p++)
            {
                if(data[p]==0) break;
                word.push_back(data[p]);
            }
            //std::cerr<<"word size "<<word.size()<<std::endl;
            oid = word.back();
            word.resize(word.size()-1);
            ++p;
            const char* pdata = (const char*)(&(data[p]));
            std::size_t len = 0;
            memcpy(&len, pdata, sizeof(std::size_t));
            if(len>0)
            {
                p+=2;
                pdata = (const char*)(&(data[p]));
                Deserializer::type izd_value(reinterpret_cast<const char*>(pdata), len);
                izd_value.read_image(*this);
            }
        }
    };
    typedef izenelib::util::izene_serialization_boost_binary<Ngram> Serializer;
    typedef izenelib::util::izene_deserialization_boost_binary<Ngram> Deserializer;
    struct NgramStat {
        NgramStat(): freq(0)
        {
        }
        word_t word;
        std::vector<std::string> text;
        std::size_t freq;
        Context left_context;
        Context right_context;
        NCIArray nci_array;
        std::size_t roid; //random oid
        std::string otitle;
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version)
        {
            ar & word & text & freq & left_context & right_context & nci_array & roid & otitle;
        }
        void clear()
        {
            word.clear();
            text.clear();
            freq = 0;
            left_context.clear();
            right_context.clear();
            nci_array.clear();
        }
        std::string GetText() const
        {
            std::stringstream ss;
            for(std::size_t i=0;i<text.size();i++)
            {
                ss<<text[i];
            }
            return ss.str();
        }
        NgramStat& operator+=(const Ngram& ngram)
        {
            if(word.empty())
            {
                word = ngram.word;
                text = ngram.text;
            }
            freq+=1;
            if(ngram.left_term!=0)
            {
                left_context.push_back(std::make_pair(ngram.left_term, 1));
            }
            if(ngram.right_term!=0)
            {
                right_context.push_back(std::make_pair(ngram.right_term, 1));
            }
            NCI nci;
            nci.cid = ngram.cid;
            nci.bid = ngram.bid;
            nci.freq = 1;
            nci.cpa.push_back(std::make_pair(ngram.capacity, ngram.price));
            nci_array.push_back(nci);
            roid = ngram.oid;
            return *this;
        }
        template <class T>
        void Accumulate_(T& value)
        {
            std::sort(value.begin(), value.end());
            T new_value;
            new_value.reserve(value.size());
            for(std::size_t i=0;i<value.size();i++)
            {
                if(new_value.empty())
                {
                    new_value.push_back(value[i]);
                }
                else
                {
                    if(value[i].first==new_value.back().first)
                    {
                        new_value.back().second+=value[i].second;
                    }
                    else
                    {
                        new_value.push_back(value[i]);
                    }
                }
            }
            std::swap(value, new_value);
        }
        void Flush()
        {
            Accumulate_(left_context);
            Accumulate_(right_context);
            {
                std::sort(nci_array.begin(), nci_array.end());
                NCIArray new_value;
                NCIArray& value = nci_array;
                new_value.reserve(value.size());
                //std::size_t pfreq=0;
                for(std::size_t i=0;i<value.size();i++)
                {
                    if(new_value.empty())
                    {
                        new_value.push_back(value[i]);
                    }
                    else
                    {
                        if(value[i].IdEqual(new_value.back()))
                        {
                            NCI& b = new_value.back();
                            b.freq += value[i].freq;
                            b.cpa.insert(b.cpa.end(), value[i].cpa.begin(), value[i].cpa.end());
                        }
                        else
                        {
                            new_value.push_back(value[i]);
                        }
                    }
                }
                for(std::size_t i=0;i<new_value.size();i++)
                {
                    NCI& nci = new_value[i];
                    std::sort(nci.cpa.begin(), nci.cpa.end());
                }
                std::swap(value, new_value);
            }
        }
    };
    typedef boost::function<bool (NgramStat& ngram)> Processor;

    CosmeticsNgram(const std::string& path): path_(path), suffix_count_(0), mid_size_(0)
    {
        if(!boost::filesystem::exists(path_))
        {
            boost::filesystem::create_directories(path_);
        }
        std::string file = path_+"/file";
        suffix_path_ = file;
        //suffix_writer_.open(file.c_str());
        suffix_writer_ = new Writer(file);
        suffix_writer_->Open();
        //dic_path_ = path_+"/dic";
        //if(boost::filesystem::exists(dic_path_))
        //{
        //    std::cerr<<"ngram dic load "<<dic_path_<<std::endl;
        //    ngram_dic_.Load(dic_path_);
        //    std::cerr<<"load finish"<<std::endl;
        //}
        //std::string cid_file = path_+"/cid";
        //if(boost::filesystem::exists(cid_file))
        //{
        //    Reader reader(cid_file);
        //    reader.Open();
        //    std::pair<term_t, std::size_t> v;
        //    while(reader.Next(v))
        //    {
        //        cid_count_[v.first] = v.second;
        //    }
        //    reader.Close();
        //}
    }



    bool Insert(const std::vector<Ngram>& ngram_list)
    {
        //std::vector<word_t> data_list;
        //for(std::size_t n=0;n<ngram_list.size();n++)
        //{
        //    const Ngram& ngram = ngram_list[n];
        //    for(std::size_t i=0;i<ngram.word.size();i++)
        //    {
        //        word_t data;
        //        if(!ngram.Serialize(i, data)) continue;
        //        if(data.empty()) continue;
        //        data_list.push_back(data);
        //    }
        //}
        //if(!data_list.empty())
        //{
        //    boost::unique_lock<boost::mutex> lock(mutex_);
        //    suffix_writer_->BatchAppend(data_list);
        //    suffix_count_+=data_list.size();
        //}
        std::vector<Ngram> data_list;
        for(std::size_t n=0;n<ngram_list.size();n++)
        {
            const Ngram& ngram = ngram_list[n];
            for(std::size_t i=0;i<ngram.word.size();i++)
            {
                Ngram data;
                if(!ngram.Pick(i, data)) continue;
                data_list.push_back(data);
            }
        }
        if(!data_list.empty())
        {
            boost::unique_lock<boost::mutex> lock(mutex_);
            for(std::size_t i=0;i<data_list.size();i++)
            {
                suffix_writer_->Append(data_list[i]);
            }
            suffix_count_+=data_list.size();
        }
        return true;
    }

    void IncCidCount(term_t cid, std::size_t count=1)
    {
        boost::unique_lock<boost::mutex> lock(cmutex_);
        TermCountMap::iterator it = cid_count_.find(cid);
        if(it==cid_count_.end()) cid_count_.insert(std::make_pair(cid, count));
        else it->second+=count;
    }

    void Finish(const Processor& processor, int thread_num=1)
    {
        suffix_writer_->Close();
        std::string cid_file = path_+"/cid";
        if(!boost::filesystem::exists(cid_file))
        {
            Writer writer(cid_file);
            writer.Open();
            for(TermCountMap::const_iterator it = cid_count_.begin();it!=cid_count_.end();++it)
            {
                std::pair<term_t, std::size_t> v(it->first, it->second);
                writer.Append(v);
            }
            writer.Close();
        }
        processor_ = processor;
        //std::size_t suffix_count = suffix_writer_->Count();
        std::string sort_file = path_+"/file2";
        if(suffix_count_>0&&!boost::filesystem::exists(sort_file))
        {
            //Sorter::Sort(suffix_path_);
            Reader reader(suffix_path_);
            reader.Open();
            std::vector<Ngram> data_list;
            data_list.reserve(reader.Count());
            LOG(INFO)<<"loading ngrams, count "<<reader.Count()<<std::endl;
            Ngram ngram;
            while( reader.Next(ngram) )
            {
                data_list.push_back(ngram);
            }
            reader.Close();
            LOG(INFO)<<"load finished, sort"<<std::endl;
            std::sort(data_list.begin(), data_list.end());
            LOG(INFO)<<"sort finished"<<std::endl;
            Writer writer(sort_file);
            writer.Open();
            for(std::size_t i=0;i<data_list.size();i++)
            {
                writer.Append(data_list[i]);
            }
            writer.Close();
        }
        //else
        //{
        //    Sorter::Sort(sort_file);
        //}
        std::string reader_file = sort_file;
        if(!boost::filesystem::exists(reader_file))
        {
            reader_file = suffix_path_;
        }
        typedef std::vector<NgramStat> Ngrams;
        std::string mid_path = path_+"/mid";
        if(boost::filesystem::exists(mid_path))
        {
            boost::filesystem::remove_all(mid_path);
        }
        mid_writer_ = new Writer(mid_path);
        mid_writer_->Open();
        B5mThreadPool<Ngrams> pool(thread_num, boost::bind(&CosmeticsNgram::Process_, this, _1));
        std::size_t p=0;
        Ngrams* to_process = new Ngrams;
        //word_t word;
        Ngram ngram;
        Reader reader(reader_file);
        reader.Open();
        std::cerr<<"reader size "<<reader.Count()<<std::endl;
        NgramStat waiter;
        while( reader.Next(ngram) )
        {
            p++;
            if(p%10000==0)
            {
                std::cerr<<"reader processing "<<p<<std::endl;
            }
            //std::cerr<<"[NGRAM]"<<ngram.word.size()<<std::endl;
            //continue;

            if(waiter.word==ngram.word)
            {
                waiter+=ngram;
            }
            else
            {
                if(!waiter.word.empty())
                {
                    if(ngram.word.front()==waiter.word.front())
                    {
                    }
                    else
                    {
                        pool.schedule(to_process);
                        to_process = new Ngrams;
                    }
                    to_process->push_back(waiter);
                }
                waiter.clear();
                waiter+=ngram;
            }
        }
        if(!to_process->empty()) pool.schedule(to_process);
        else delete to_process;
        pool.wait();
        reader.Close();
        mid_writer_->Close();
        delete mid_writer_;
    }

    static const uint32_t GetContextLength()
    {
        return Ngram::GetContextLength();
    }

private:
    void Process_(std::vector<NgramStat>& input)
    {
        for(std::size_t i=0;i<input.size();i++)
        {
            const NgramStat& base_ngram = input[i];
            std::pair<std::size_t, std::size_t> range(1, base_ngram.word.size());
            if(i>0)
            {
                const NgramStat& prev = input[i-1];
                std::size_t common_prefix_len = 0;
                for(std::size_t r=0;r<prev.word.size();r++)
                {
                    if(prev.word[r]!=base_ngram.word[r])
                    {
                        break;
                    }
                    ++common_prefix_len;
                }
                range.first = common_prefix_len+1;
            }
            for(std::size_t r=range.first;r<=range.second;r++)
            {
                NgramStat ngram;
                ngram.word.assign(base_ngram.word.begin(), base_ngram.word.begin()+r);
                ngram.text.assign(base_ngram.text.begin(), base_ngram.text.begin()+r);
                ngram.roid = base_ngram.roid;
                for(std::size_t j=i;j<input.size();j++)
                {
                    const NgramStat& ngramj = input[j];
                    if(boost::algorithm::starts_with(ngramj.word, ngram.word))
                    {
                        ngram.freq+=ngramj.freq;
                        std::size_t right_context_len = std::min(ngramj.word.size()-ngram.word.size(), (std::size_t)GetContextLength());
                        if(right_context_len>0)
                        {
                            //word_t right_context(ngramj.word.begin()+ngram.word.size(), ngramj.word.begin()+ngram.word.size()+right_context_len);
                            //ngram.right_context.push_back(std::make_pair(right_context, ngramj.freq));
                            term_t right_term = ngramj.word[ngram.word.size()];
                            ngram.right_context.push_back(std::make_pair(right_term, ngramj.freq));
                        }
                        ngram.left_context.insert(ngram.left_context.end(), ngramj.left_context.begin(), ngramj.left_context.end());
                        ngram.nci_array.insert(ngram.nci_array.end(), ngramj.nci_array.begin(), ngramj.nci_array.end());
                    }
                    else
                    {
                        break;
                    }
                }
                ngram.Flush();
                uint32_t max_lc = 0;
                uint32_t max_rc = 0;
                for(std::size_t i=0;i<ngram.left_context.size();i++)
                {
                    if(ngram.left_context[i].second>max_lc)
                    {
                        max_lc = ngram.left_context[i].second;
                    }
                }
                for(std::size_t i=0;i<ngram.right_context.size();i++)
                {
                    if(ngram.right_context[i].second>max_rc)
                    {
                        max_rc = ngram.right_context[i].second;
                    }
                }
                double max_lc_prop = (double)max_lc/ngram.freq;
                double max_rc_prop = (double)max_rc/ngram.freq;
                if(max_lc_prop>=0.9||max_rc_prop>=0.9) continue;
                TryOutput_(ngram);
            }
        }
    }

    void TryOutput_(NgramStat& ngram)
    {
        if(!processor_(ngram)) return;
        boost::unique_lock<boost::mutex> lock(mutex_);
        //std::cerr<<"[NGRAM]"<<ngram.GetText()<<","<<ngram.dfreq<<","<<ngram.nci_array.size()<<","<<ngram.left_context.size()<<","<<ngram.right_context.size()<<std::endl;
        mid_writer_->Append(ngram);
        mid_size_++;
        if(mid_size_%100000==0)
        {
            std::cerr<<"[MID-SIZE]"<<mid_size_<<std::endl;
        }
    }


private:
    std::string path_;
    std::string dic_path_;
    std::string suffix_path_;
    std::size_t suffix_count_;
    Writer* suffix_writer_;
    Writer* mid_writer_;
    std::size_t mid_size_;
    Processor processor_;
    TermCountMap cid_count_;
    boost::mutex mutex_;
    boost::mutex cmutex_;
};

NS_IDMLIB_B5M_END

#endif

