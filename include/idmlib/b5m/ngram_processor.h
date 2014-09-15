#ifndef IDMLIB_B5M_NGRAMPROCESSOR_H_
#define IDMLIB_B5M_NGRAMPROCESSOR_H_
#include "b5m_helper.h"
#include "b5m_types.h"
#include "product_matcher.h"
#include "ngram_dictionary.h"
#include "scd_doc_processor.h"
#include <am/sequence_file/ssfr.h>
#include <sf1common/ScdWriter.h>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <util/izene_serialization.h>
//#define NGRAMP_DEBUG

NS_IDMLIB_B5M_BEGIN

using izenelib::util::UString;

class NgramProcessor {

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
    typedef WordCountArray Context;
    typedef boost::unordered_map<term_t, std::size_t> TermCountMap;
    typedef boost::unordered_map<term_t, score_t> CategoryScore;
    typedef boost::unordered_map<term_t, std::size_t> CategoryCount;
    struct NCI
    {
        NCI() :cid(0), freq(0), price(0.0), prob(0.0)
        {
        }
        term_t cid;
        std::size_t freq;
        double price;
        double prob;
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version)
        {
            ar & cid & freq & price & prob;
        }
        bool operator<(const NCI& y) const
        {
            return cid<y.cid;
        }
    };
    typedef std::vector<NCI> NCIArray;
    typedef boost::unordered_map<term_t, NCI> CategoryNCI;
    struct NgramSer
    {
        std::vector<std::string> text;
        Context left_context;
        NCIArray nci_array;
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version)
        {
            ar & text & left_context & nci_array;
        }
    };
    typedef izenelib::util::izene_serialization_boost_binary<NgramSer> Serializer;
    typedef izenelib::util::izene_deserialization_boost_binary<NgramSer> Deserializer;
    struct Ngram
    {
        word_t word;
        std::vector<std::string> text;
        term_t type; //title(0) or original category(1)
        term_t oid;
        std::size_t freq;
        std::size_t dfreq;
        Context left_context;
        Context right_context;
        NCIArray nci_array;
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version)
        {
            ar & word & text & type & oid & freq & dfreq & left_context & right_context & nci_array;
        }
        Ngram(): type(0), oid(0), freq(0), dfreq(0)
        {
        }
        void insert(const NCI& v)
        {
            nci_array.push_back(v);
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
                std::size_t pfreq=0;
                for(std::size_t i=0;i<value.size();i++)
                {
                    if(new_value.empty())
                    {
                        new_value.push_back(value[i]);
                    }
                    else
                    {
                        if(value[i].cid==new_value.back().cid)
                        {
                            NCI& b = new_value.back();
                            b.freq += value[i].freq;
                            if(value[i].price>0.0)
                            {
                                b.price += value[i].price;
                            }
                        }
                        else
                        {
                            if(pfreq>0)
                            {
                                NCI& b = new_value.back();
                                b.price /= pfreq;
                                pfreq = 0;
                            }
                            new_value.push_back(value[i]);
                        }
                    }
                    if(value[i].price>0.0) pfreq++;
                }
                if(pfreq>0)
                {
                    NCI& b = new_value.back();
                    b.price /= pfreq;
                    pfreq = 0;
                }
                std::swap(value, new_value);
            }
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
        static const uint32_t GetContextLength()
        {
            static const uint32_t l = 1;
            return l;
        }
        bool Serialize(word_t& data, uint32_t index) const
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
            NgramSer ns;
            if(index<text.size())
            {
                ns.text.assign(text.begin()+index, text.end());
            }
            ns.nci_array = nci_array;
            
            int64_t li = index;
            WordCount ns_left_context_item;
            for(uint32_t i=0;i<GetContextLength();i++)
            {
                li--;
                if(li>=0)
                {
                    ns_left_context_item.first.push_back(word[li]);
                    ns_left_context_item.second = freq;
                }
                else
                {
                    uint64_t pli = li*(-1);
                    if(pli<left_context.size())
                    {
                        ns_left_context_item.first.push_back(left_context.front().first[pli]);
                        ns_left_context_item.second = left_context.front().second;
                    }
                    else
                    {
                        break;
                    }
                }
            }
            if(!ns_left_context_item.first.empty())
            {
                ns.left_context.push_back(ns_left_context_item);
#ifdef NGRAMP_DEBUG
                std::cerr<<"[LEFT]";
                for(std::size_t i=0;i<ns_left_context_item.first.size();i++)
                {
                    std::cerr<<ns_left_context_item.first[i]<<",";
                }
                std::cerr<<"\t:"<<ns_left_context_item.second<<std::endl;
#endif
            }
            else
            {
#ifdef NGRAMP_DEBUG
                std::cerr<<"[LEFT-EMPTY]"<<std::endl;
#endif
            }
            char* ptr;
            std::size_t len;
            Serializer::type izs(ns);
            izs.write_image(ptr, len);
            std::size_t prop = sizeof(term_t);
            std::size_t plen = (word.size()-index+1+2+2)*sizeof(term_t)+len;
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
            memcpy(pdata, &type, step);
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
            type = word[word.size()-2];
            oid = word.back();
            word.resize(word.size()-2);
            ++p;
            const char* pdata = (const char*)(&(data[p]));
            std::size_t len = 0;
            memcpy(&len, pdata, sizeof(std::size_t));
            if(len>0)
            {
                p+=2;
                pdata = (const char*)(&(data[p]));
                NgramSer ns;
                Deserializer::type izd_value(reinterpret_cast<const char*>(pdata), len);
                izd_value.read_image(ns);
                text = ns.text;
                nci_array = ns.nci_array;
                left_context = ns.left_context;
            }
        }
    };
    struct NgramValue
    {
        typedef CategoryNCI::const_iterator const_iterator;
        typedef CategoryNCI::iterator iterator;
        NgramValue(): type(0), weight(1.0)
        {
        }

        NgramValue(const Ngram& ngram): 
            //word(ngram.word), text(ngram.text), 
            //type(ngram.type), freq(ngram.dfreq), category_distrib(ngram.category_freq.size())
            word(ngram.word), 
            type(ngram.type),
            weight(1.0)
        {
            for(std::size_t i=0;i<ngram.nci_array.size();i++)
            {
                value[ngram.nci_array[i].cid] = ngram.nci_array[i];
                //category_distrib[i].first = ngram.category_freq[i].first;
                //category_distrib[i].second = (double)ngram.category_freq[i].second;
            }
        }
        std::string GetText() const
        {
            std::stringstream ss;
            //for(std::size_t i=0;i<text.size();i++)
            //{
            //    ss<<text[i];
            //}
            return ss.str();
        }

        const_iterator begin() const
        {
            return value.begin();
        }

        const_iterator end() const
        {
            return value.end();
        }

        const_iterator find(term_t cid) const
        {
            return value.find(cid);
        }

        iterator begin()
        {
            return value.begin();
        }
        iterator end()
        {
            return value.end();
        }

        iterator find(term_t cid)
        {
            return value.find(cid);
        }

        void insert(const NCI& v)
        {
            value.insert(std::make_pair(v.cid, v));
        }

        //NCI& operator[](term_t cid)
        //{
        //    return value[cid];
        //}
        //const NCI& operator[](term_t cid) const
        //{
        //    return const_cast<NCI&>((*this)[cid]);
        //}


        word_t word;
        //std::string text;
        term_t type;
        //std::vector<std::string> text;
        CategoryNCI value;
        double weight;
        //std::size_t freq;
        //TermScoreArray category_distrib;
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version)
        {
            ar 
              //& word & text 
              //& type & freq & category_distrib;
              & word & type & value;
        }

    };
    typedef boost::function<bool (Ngram& ngram)> Processor;
    typedef NgramDictionary<NgramValue> NgramDic;

    NgramProcessor(const std::string& path): path_(path), max_context_prop_(0.9), suffix_count_(0), mid_size_(0)
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
        dic_path_ = path_+"/dic";
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

    void SetMaxContextProp(double v)
    {
        max_context_prop_ = v;
    }

    NgramDic& GetNgramDic()
    {
        return ngram_dic_;
    }

    static void ApplyCategoryCount(NgramValue& nv, const std::vector<Category>& category_list)
    {
        CategoryCount additional;
        CategoryScore price_sum;
        for(NgramValue::const_iterator it = nv.begin();it!=nv.end();++it)
        {
            term_t cid = it->first;
            const NCI& score = it->second;
            double psum = score.price*score.freq;
            CategoryScore::iterator sit = price_sum.find(cid);
            if(sit==price_sum.end()) price_sum.insert(std::make_pair(cid, psum));
            else sit->second+=psum;
            while(true)
            {
                cid = category_list[cid].parent_cid;
                CategoryCount::iterator it = additional.find(cid);
                if(it==additional.end()) additional.insert(std::make_pair(cid, score.freq));
                else it->second += score.freq;
                sit = price_sum.find(cid);
                if(sit==price_sum.end()) price_sum.insert(std::make_pair(cid, psum));
                else sit->second+=psum;
                if(cid==0) break;
            }
        }
        for(NgramValue::iterator it = nv.begin();it!=nv.end();++it)
        {
            term_t cid = it->first;
            NCI& score = it->second;
            CategoryCount::iterator it = additional.find(cid);
            if(it!=additional.end())
            {
                score.freq += it->second;
                additional.erase(it);
            }
        }
        for(CategoryCount::const_iterator it=additional.begin();it!=additional.end();++it)
        {
            NCI value;
            value.cid = it->first;
            value.freq = it->second;
            //TODO price
            nv.insert(value);
            //nv.category_distrib.push_back(std::make_pair(it->first, it->second));
        }
        for(NgramValue::iterator it = nv.begin();it!=nv.end();++it)
        {
            term_t cid = it->first;
            NCI& v = it->second;
            double psum = 0.0;
            CategoryScore::iterator sit = price_sum.find(cid);
            if(sit!=price_sum.end()) psum = sit->second;
            if(v.freq>0)
            {
                v.price = psum/v.freq;
            }
        }

    }

    //static void CalculateCategoryProb(NgramValue& nv, const std::vector<Category>& category_list, term_t pcid)
    //{
    //    const Category& pc = category_list[pcid];
    //    const std::vector<term_t>& target_cids = pc.children;
    //    NgramValue::const_iterator pit = nv.find(pcid);
    //    if(pit==nv.end()) return;
    //    const NCI& pnci = pit->second;
    //    for(std::size_t j=0;j<target_cids.size();j++)
    //    {
    //        term_t cid = target_cids[j];
    //        NgramValue::iterator it = nv.find(cid);
    //        if(it==nv.end()) continue;
    //        NCI& nci = it->second;
    //        std::size_t count = nci.freq;
    //        std::size_t pcount = pnci.freq;
    //        const Category& c = category_list[cid];
    //        double rscore = 0.0;
    //        if(pc.offer_count>c.offer_count)
    //        {
    //            //std::cerr<<"pc c offer_count "<<pc.offer_count<<","<<c.offer_count<<std::endl;
    //            rscore = (double)(pcount-count)/(pc.offer_count-c.offer_count);
    //        }
    //        double score = (double)count/c.offer_count;
    //        score -= rscore;
    //        nci.prob = score;
    //    }
    //}
    static void CalculateCategoryProb(NgramValue& nv, const std::vector<Category>& category_list)
    {
        for(NgramValue::iterator it = nv.begin();it!=nv.end();++it)
        {
            term_t cid = it->first;
            if(cid==0) continue;
            NCI& nci = it->second;
            std::size_t count = nci.freq;
            const Category& c = category_list[cid];
            const Category& pc = category_list[c.parent_cid];
            std::size_t pcount = count;
            NgramValue::const_iterator pit = nv.find(c.parent_cid);
            if(pit==nv.end())
            {
                std::cerr<<"!!!!!err "<<cid<<","<<c.parent_cid<<std::endl;
            }
            else
            {
                pcount = pit->second.freq;
            }
            double rscore = 0.0;
            if(pc.offer_count>c.offer_count)
            {
                //std::cerr<<"pc c offer_count "<<pc.offer_count<<","<<c.offer_count<<std::endl;
                rscore = (double)(pcount-count)/(pc.offer_count-c.offer_count);
            }
            double score = (double)count/c.offer_count;
            score -= rscore;
            nci.prob = score;
        }
    }
    //does not minus the prob of sibling
    static void CalculateCategoryProb2(NgramValue& nv, const std::vector<Category>& category_list)
    {
        for(NgramValue::iterator it = nv.begin();it!=nv.end();++it)
        {
            term_t cid = it->first;
            if(cid==0) continue;
            NCI& nci = it->second;
            std::size_t count = nci.freq;
            const Category& c = category_list[cid];
            //const Category& pc = category_list[c.parent_cid];
            std::size_t pcount = count;
            NgramValue::const_iterator pit = nv.find(c.parent_cid);
            if(pit==nv.end())
            {
                std::cerr<<"!!!!!err "<<cid<<","<<c.parent_cid<<std::endl;
            }
            else
            {
                pcount = pit->second.freq;
            }
            double rscore = 0.0;
            //if(pc.offer_count>c.offer_count)
            //{
            //    //std::cerr<<"pc c offer_count "<<pc.offer_count<<","<<c.offer_count<<std::endl;
            //    rscore = (double)(pcount-count)/(pc.offer_count-c.offer_count);
            //}
            double score = (double)count/c.offer_count;
            score -= rscore;
            nci.prob = score;
        }
    }


    bool Insert(const std::vector<Ngram>& ngram_list)
    {
        std::vector<word_t> data_list;
        for(std::size_t n=0;n<ngram_list.size();n++)
        {
            const Ngram& ngram = ngram_list[n];
            for(std::size_t i=0;i<ngram.word.size();i++)
            {
                word_t data;
                if(!ngram.Serialize(data, i)) continue;
                if(data.empty()) continue;
                data_list.push_back(data);
                //const char* pdata = (const char*)&(data[0]);
                //std::size_t plen = data.size()*sizeof(term_t);
                //std::string str(pdata, plen);
                //suffix_writer_<<str<<std::endl;
            }
        }
        if(!data_list.empty())
        {
            boost::unique_lock<boost::mutex> lock(mutex_);
            suffix_writer_->BatchAppend(data_list);
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

    void Finish(const Processor& processor)
    {
        //suffix_writer_.close();
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
            Sorter::Sort(suffix_path_);
            //std::string cmd = "env LC_COLLATE=C /home/ops/coreutils/bin/sort --buffer-size=30% "+suffix_path_+" > "+sort_file;
            //LOG(INFO)<<"cmd : "<<cmd<<std::endl;
            //int status = system(cmd.c_str());
            //LOG(INFO)<<"cmd finished : "<<status<<std::endl;
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
        typedef std::vector<Ngram> Ngrams;
        //std::vector<Ngram> to_process;
        std::string mid_path = path_+"/mid";
        if(boost::filesystem::exists(mid_path))
        {
            boost::filesystem::remove_all(mid_path);
        }
        mid_writer_ = new Writer(mid_path);
        mid_writer_->Open();
        B5mThreadPool<Ngrams> pool(20, boost::bind(&NgramProcessor::Process_, this, _1));
        std::size_t p=0;
        std::vector<Ngram> waiters;
        waiters.resize(3);
        std::vector<Ngrams*> to_process;
        to_process.push_back(NULL);
        to_process.push_back(new Ngrams);
        to_process.push_back(new Ngrams);
        //Ngrams* to_process = new Ngrams;
        //Ngram waiter;
        word_t word;
        //std::ifstream ifs(reader_file.c_str());
        //std::string line;
        //while(getline(ifs, line))
        Reader reader(reader_file);
        reader.Open();
        std::cerr<<"reader size "<<reader.Count()<<std::endl;
        while( reader.Next(word) )
        {
            p++;
            if(p%100000==0)
            {
                std::cerr<<"reader processing "<<p<<std::endl;
            }
            //word_t word;
            //const char* pdata = line.c_str();
            //std::size_t plen = line.length()/4;
            //const term_t* pstart = (const term_t*)pdata;
            //const term_t* pend = pstart+plen;
            //word.assign(pstart, pend);
            Ngram ngram;
            ngram.Deserialize(word);
            term_t type = ngram.type;
            Ngram& waiter = waiters[type];

            //std::cerr<<"[DEBUG]"<<ngram.type<<std::endl;
            //continue;
            if(waiter.word==ngram.word)
            {
                waiter.freq+=1;
                waiter.left_context.insert(waiter.left_context.end(), ngram.left_context.begin(), ngram.left_context.end());
                if(ngram.oid!=waiter.oid) 
                {
                    waiter.dfreq+=1;
                    waiter.nci_array.insert(waiter.nci_array.end(), ngram.nci_array.begin(), ngram.nci_array.end());
                }
                waiter.oid = ngram.oid;
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
                        pool.schedule(to_process[type]);
                        to_process[type] = new Ngrams;
                    }
                    to_process[type]->push_back(waiter);
                }
                waiter = ngram;
                waiter.freq = 1;
                waiter.dfreq = 1;
            }
        }
        for(std::size_t i=1;i<=2;i++)
        {
            if(!to_process[i]->empty())
            {
                pool.schedule(to_process[i]);
            }
            else
            {
                delete to_process[i];
            }
        }
        pool.wait();
        reader.Close();
        mid_writer_->Close();
        delete mid_writer_;
        std::cerr<<"start to flush dic on "<<dic_path_<<std::endl;
        ngram_dic_.Flush(dic_path_);
        std::cerr<<"flush finish"<<std::endl;
    }

    void DicTest(const word_t& key, term_t type, std::vector<NgramValue>& values)
    {
        ngram_dic_.Search(key, type, values);
    }

    static const uint32_t GetContextLength()
    {
        return Ngram::GetContextLength();
    }

private:
    void Process_(std::vector<Ngram>& input)
    {
        for(std::size_t i=0;i<input.size();i++)
        {
            const Ngram& base_ngram = input[i];
            //std::cerr<<"[DEBUGTYPE]"<<i<<","<<base_ngram.type<<std::endl;
            std::pair<std::size_t, std::size_t> range(1, base_ngram.word.size());
            if(i>0)
            {
                const Ngram& prev = input[i-1];
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
            //std::stack<Ngram*> stack;
            for(std::size_t r=range.first;r<=range.second;r++)
            {
                //Ngram* pngram = new Ngram;
                //Ngram& ngram = *pngram;
                Ngram ngram;
                ngram.type = base_ngram.type;
                ngram.word.assign(base_ngram.word.begin(), base_ngram.word.begin()+r);
                if(base_ngram.text.size()>=r)
                {
                    ngram.text.assign(base_ngram.text.begin(), base_ngram.text.begin()+r);
                }
                for(std::size_t j=i;j<input.size();j++)
                {
                    const Ngram& ngramj = input[j];
                    if(boost::algorithm::starts_with(ngramj.word, ngram.word))
                    {
                        ngram.freq+=ngramj.freq;
                        ngram.dfreq+=ngramj.dfreq;
                        std::size_t right_context_len = std::min(ngramj.word.size()-ngram.word.size(), (std::size_t)GetContextLength());
                        if(right_context_len>0)
                        {
                            word_t right_context(ngramj.word.begin()+ngram.word.size(), ngramj.word.begin()+ngram.word.size()+right_context_len);
                            ngram.right_context.push_back(std::make_pair(right_context, ngramj.freq));
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
                if(max_lc_prop>max_context_prop_||max_rc_prop>max_context_prop_) continue;
                TryOutput_(ngram);
                //if(stack.empty())
                //{
                //    stack.push(pngram);
                //}
                //else
                //{
                //    const Ngram* ptop = stack.top();
                //    const Ngram& top = *(ptop);
                //    double prop = (double)ngram.freq/top.freq;
                //    if(prop>=0.9)
                //    {
                //        stack.pop();
                //        delete ptop;
                //    }
                //    stack.push(pngram);
                //}
            }
            //std::stack<Ngram*> stack2;
            //while(!stack.empty())
            //{
            //    Ngram* ptop = stack.top();
            //    stack2.push(ptop);
            //    stack.pop();
            //}
            //while(!stack2.empty())
            //{
            //    const Ngram* ptop = stack2.top();
            //    const Ngram& top = *ptop;
            //    TryOutput_(top);
            //    stack2.pop();
            //    delete ptop;
            //}
        }
    }

    void TryOutput_(Ngram& ngram)
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
        //NgramValue nv(ngram);
        //double sum = 0.0;
        //for(std::size_t i=0;i<nv.category_distrib.size();i++)
        //{
        //    term_t cid = ngram.category_freq[i].first;
        //    nv.category_distrib[i].first = cid;
        //    double prop = (double)ngram.category_freq[i].second/std::log(cid_count_[cid]+2);
        //    nv.category_distrib[i].second = prop;
        //    sum+=prop;
        //}
        //for(std::size_t i=0;i<nv.category_distrib.size();i++)
        //{
        //    nv.category_distrib[i].second/=sum;
        //}
        //ngram_dic_.Insert(nv.word, nv);
        //if(ngram_dic_.Size()%10000==0)
        //{
        //    std::cerr<<"[DIC-SIZE]"<<ngram_dic_.Size()<<std::endl;
        //}
    }


private:
    std::string path_;
    std::string dic_path_;
    double max_context_prop_;
    std::string suffix_path_;
    std::size_t suffix_count_;
    //std::ofstream suffix_writer_;
    Writer* suffix_writer_;
    Writer* mid_writer_;
    std::size_t mid_size_;
    Processor processor_;
    NgramDic ngram_dic_;
    TermCountMap cid_count_;
    boost::mutex mutex_;
    boost::mutex cmutex_;
};

NS_IDMLIB_B5M_END

#endif

