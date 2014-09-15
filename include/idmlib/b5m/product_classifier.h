#ifndef IDMLIB_B5M_PRODUCTCLASSIFIER_H_
#define IDMLIB_B5M_PRODUCTCLASSIFIER_H_
#include "b5m_helper.h"
#include "b5m_types.h"
#include "product_matcher.h"
#include "ngram_processor.h"
#include <sf1common/ScdWriter.h>
#include <idmlib/maxent/maxentmodel.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/mersenne_twister.hpp>

NS_IDMLIB_B5M_BEGIN

using izenelib::util::UString;

class ProductClassifier {
    typedef uint32_t term_t;
    typedef double score_t;
    typedef std::vector<term_t> word_t;
    typedef std::vector<word_t> sentence_t;
    struct Word {
        word_t word;
        std::vector<std::string> text;
    };
    typedef std::vector<Word> Sentence;
    typedef std::pair<term_t, std::size_t> TermCount;
    typedef std::pair<word_t, std::size_t> WordCount;
    typedef std::vector<TermCount> TermCountArray;
    typedef std::vector<WordCount> WordCountArray;
    typedef WordCountArray Context;
    typedef boost::unordered_map<std::string, term_t> CategoryIndex;
    typedef boost::unordered_map<term_t, std::string> CategoryText;
    typedef boost::unordered_map<term_t, double> CategoryScore;
    typedef std::pair<term_t, score_t> TermScore;
    typedef std::vector<TermScore> TermScoreArray;
    typedef boost::unordered_map<term_t, std::size_t> CategoryCount;
    typedef izenelib::am::ssf::Reader<std::size_t> Reader;
    typedef izenelib::am::ssf::Writer<std::size_t> Writer;
    typedef izenelib::am::ssf::Sorter<std::size_t, uint32_t, true> JointSorter;
    typedef std::pair<boost::atomic<std::size_t>, boost::atomic<std::size_t> > AtomicPair;
    struct AttributePostingItem
    {
        term_t cid;
        term_t spu_id;
        std::string attribute_name;
        bool is_optional;
    };
    struct AttributeMapValue
    {
        std::string name;
        std::vector<AttributePostingItem> attribute_posting;
        void Compute(const std::vector<Category>& category_list)
        {
            std::vector<AttributePostingItem> new_posting;
            std::size_t osize = attribute_posting.size();
            for(std::size_t i=0;i<osize;i++)
            {
                const AttributePostingItem& ai = attribute_posting[i];
                term_t cid = ai.cid;
                while(true)
                {
                    cid = category_list[cid].parent_cid;
                    if(cid==0) break;
                    AttributePostingItem oai(ai);
                    oai.cid = cid;
                    attribute_posting.push_back(oai);
                }
            }
        }
    };
    typedef std::map<word_t, AttributeMapValue> AttributeMap;
    typedef NgramProcessor::Ngram Ngram;
    typedef NgramProcessor::NgramValue NgramValue;
    typedef NgramProcessor::NCI NCI;
    typedef NgramProcessor::NCIArray NCIArray;
    typedef NgramDictionary<NgramValue> NgramDic;
    typedef boost::unordered_map<word_t, NgramValue> JointDic;
    struct AnalyzeItem {
        AnalyzeItem(): price(0.0)
        {
        }
        Document doc;
        std::vector<NgramValue> tvalues;
        std::vector<NgramValue> ocvalues;
        double price;
    };

    struct MaxentClassifier {

        void Load(const std::string& model)
        {
            me.load(model);
        }

        void Evaluate(const std::vector<term_t>& id_list, CategoryScore& result)
        {
            std::vector<std::pair<std::string, float> > contexts(id_list.size());
            for(std::size_t i=0;i<id_list.size();i++)
            {
                contexts[i].first = boost::lexical_cast<std::string>(id_list[i]);
                contexts[i].second = 1.0f;
            }
            std::vector<std::pair<std::string, double> > me_results;
            me.eval_all(contexts, me_results);
            for(std::size_t i=0;i<me_results.size();i++)
            {
                term_t cid = boost::lexical_cast<term_t>(me_results[i].first);
                result[cid] = me_results[i].second;
            }
        }

    private:
        maxent::MaxentModel me;

    };

public:
    ProductClassifier(): analyzer_(NULL), oid_(1), ngram_processor_(NULL), dic_(NULL), joint_dic_(NULL)
                         , dic_size_(0) , p_(0), lastp_(0), time_(0.0)
    {
        recall_.push_back(NULL);
        for(std::size_t i=0;i<3;i++)
        {
            AtomicPair* v = new AtomicPair;
            v->first = 0;
            v->second = 0;
            recall_.push_back(v);
        }
        accuracy_.push_back(NULL);
        for(std::size_t i=0;i<3;i++)
        {
            AtomicPair* v = new AtomicPair;
            v->first = 0;
            v->second = 0;
            accuracy_.push_back(v);
        }
        idmlib::util::IDMAnalyzerConfig csconfig = idmlib::util::IDMAnalyzerConfig::GetCommonConfig("","", "");
        csconfig.symbol = true;
        analyzer_ = new idmlib::util::IDMAnalyzer(csconfig);
    }

    ~ProductClassifier()
    {
        if(analyzer_!=NULL) delete analyzer_;
        for(std::size_t i=0;i<accuracy_.size();i++)
        {
            if(accuracy_[i]!=NULL) delete accuracy_[i];
        }
        for(std::size_t i=0;i<recall_.size();i++)
        {
            if(recall_[i]!=NULL) delete recall_[i];
        }
        for(std::size_t i=0;i<me_classifiers_.size();i++)
        {
            if(me_classifiers_[i]!=NULL) delete me_classifiers_[i];
        }
    }

    void TryDicInsert_(Ngram& ngram)
    {
        //std::cerr<<"[KEYTYPE]"<<ngram.type<<std::endl;
        NgramValue nv(ngram);
        nv.word = ngram.word;
        NgramProcessor::ApplyCategoryCount(nv, category_list_);
        if(!NgramFilter2_(nv, ngram.GetText())) return;
        word_t key(1, nv.type);
        key.insert(key.end(), nv.word.begin(), nv.word.end());
        NgramProcessor::CalculateCategoryProb(nv, category_list_);
        boost::unique_lock<boost::mutex> lock(mutex_);
        dic_writer_->Append(nv);
        //dic_->Insert(key, nv);
        dic_size_++;
    }
    void TryDicInsertDirectly_(Ngram& ngram)
    {
        //std::cerr<<"[KEYTYPE]"<<ngram.type<<std::endl;
        NgramValue nv(ngram);
        nv.word = ngram.word;
        NgramProcessor::ApplyCategoryCount(nv, category_list_);
        if(!NgramFilter2_(nv, ngram.GetText())) return;
        word_t key(1, nv.type);
        key.insert(key.end(), nv.word.begin(), nv.word.end());
        NgramProcessor::CalculateCategoryProb(nv, category_list_);
        boost::unique_lock<boost::mutex> lock(mutex_);
        //dic_writer_->Append(nv);
        dic_->Insert(key, nv);
        dic_size_++;
    }
    void TryJointDicInsert_(Ngram& ngram)
    {
        //std::cerr<<"[KEYTYPE]"<<ngram.type<<std::endl;
        NgramValue nv(ngram);
        nv.word = ngram.word;
        NgramProcessor::ApplyCategoryCount(nv, category_list_);
        if(!DistributionFilter_(nv)) return;
        NgramProcessor::CalculateCategoryProb(nv, category_list_);
        boost::unique_lock<boost::mutex> lock(mutex_);
        joint_dic_->insert(std::make_pair(nv.word, nv));
        dic_size_++;
    }
    void TryDicInsert2_(Ngram& ngram)
    {
        //std::cerr<<"[KEYTYPE]"<<ngram.type<<std::endl;
        NgramValue nv(ngram);
        nv.word = ngram.word;
        NgramProcessor::ApplyCategoryCount(nv, category_list_);
        if(!NgramFilter2_(nv, ngram.GetText())) return;
        word_t key(1, nv.type);
        key.insert(key.end(), nv.word.begin(), nv.word.end());
        //does not minus sibling
        NgramProcessor::CalculateCategoryProb2(nv, category_list_);
        boost::unique_lock<boost::mutex> lock(mutex_);
        dic_size_++;
        //nv.id = dic_size_;
        dic_->Insert(key, nv);
    }

    bool Load(const std::string& resource_dir)
    {
        std::string work_dir = resource_dir+"/work_dir";
        std::string mid_file = work_dir+"/mid";
        std::string dic_file= work_dir+"/dic";
        std::string file2 = work_dir+"/file2";
        std::string cid_file= work_dir+"/cid";
        std::string maxent_done = work_dir+"/maxent/done";
        if(boost::filesystem::exists(cid_file))
        {
            Reader reader(cid_file);
            reader.Open();
            std::pair<term_t, std::size_t> v;
            while(reader.Next(v))
            {
                category_list_[v.first].offer_count = v.second;
            }
            reader.Close();
            for(std::size_t i=1;i<category_list_.size();i++)
            {
                Category* c = &(category_list_[i]);
                std::size_t count = c->offer_count;
                while(true)
                {
                    Category* pc = &(category_list_[c->parent_cid]);
                    pc->offer_count += count;
                    c = pc;
                    if(c->cid==0) break;
                }
            }
        }
        if(boost::filesystem::exists(maxent_done))
        {
            me_classifiers_.resize(category_list_.size(), NULL);
            for(std::size_t i=0;i<category_list_.size();i++)
            {
                const Category& c = category_list_[i];
                LOG(INFO)<<"[CATEGORY]"<<i<<","<<c.name<<std::endl;
                std::string model_file = work_dir+"/maxent/"+boost::lexical_cast<std::string>(i)+".memodel";
                if(boost::filesystem::exists(model_file))
                {
                    LOG(INFO)<<"Loading me model file "<<model_file<<std::endl;
                    me_classifiers_[i] = new MaxentClassifier;
                    me_classifiers_[i]->Load(model_file);
                }
            }
        }
        if(boost::filesystem::exists(dic_file))
        {
            //ngram_processor_ = new NgramProcessor(work_dir);
            Reader reader(dic_file);
            reader.Open();
            std::cerr<<"dic count "<<reader.Count()<<std::endl;
            NgramValue nv;
            dic_ = new NgramDic;
            std::size_t p=0;
            while(reader.Next(nv))
            {
                p++;
                if(p%100000==0)
                {
                    std::cerr<<"load "<<p<<" items"<<std::endl;
                }
                //std::cerr<<"[KEYL]"<<nv.word.size()<<","<<nv.type<<","<<nv.value.size()<<std::endl;
                word_t key(1, nv.type);
                key.insert(key.end(), nv.word.begin(), nv.word.end());
                dic_->Insert(key, nv);
            }
            std::cerr<<"load dic finish "<<dic_->Size()<<std::endl;
            reader.Close();
            return true;
        }
        else if(boost::filesystem::exists(mid_file))
        {
            ngram_processor_ = new NgramProcessor(work_dir);
            dic_ = &(ngram_processor_->GetNgramDic());
            //ngram_processor_->ApplyCategory(category_list_);
            NgramProcessor::Reader reader(mid_file);
            reader.Open();
            std::cerr<<"TOTAL COUNT: "<<reader.Count()<<std::endl;
            B5mThreadPool<Ngram> pool(10, boost::bind(&ProductClassifier::TryDicInsert_, this, _1));
            Ngram* ngram = new Ngram;
            std::size_t p=0;
            dic_writer_ = new Writer(dic_file);
            dic_writer_->Open();
            while(reader.Next( (*ngram) ))
            {
                ++p;
                if(p%100000==0)
                {
                    std::cerr<<"mid loading "<<p<<","<<dic_size_<<std::endl;
                }
                pool.schedule(ngram);
                ngram = new Ngram;
            }
            reader.Close();
            pool.wait();
            dic_writer_->Close();
            delete dic_writer_;
            return true;
        }
        else if(boost::filesystem::exists(file2))
        {
            //if(boost::filesystem::exists(mid_file))
            //{
            //    boost::filesystem::remove_all(mid_file);
            //}
            ngram_processor_ = new NgramProcessor(work_dir);
            dic_ = &(ngram_processor_->GetNgramDic());
            ngram_processor_->Finish(boost::bind(&ProductClassifier::NgramProcess_, this, _1));
            return true;
        }
        return false;
    }

    bool Train(const std::string& dir)
    {
        //return MaxentTrain(dir);
        std::string category_file = dir+"/category";
        LoadCategoryInfo_(category_file);
        std::string spu_scd = dir+"/SPU.SCD";
        {
            ScdDocProcessor::ProcessorType p = boost::bind(&ProductClassifier::SpuInsert_, this, _1);
            ScdDocProcessor sd_processor(p, 1);
            sd_processor.AddInput(spu_scd);
            sd_processor.Process();
            for(std::size_t i=1;i<category_list_.size();i++)
            {
                const Category& c = category_list_[i];
                const std::vector<std::string>& keywords = c.keywords;
                for(std::size_t k=0;k<keywords.size();k++)
                {
                    word_t word;
                    GetWord_(keywords[k], word);
                    AttributeMapValue& v = attribute_map_[word];
                    v.name = keywords[k];
                    AttributePostingItem item;
                    item.cid = c.cid;
                    item.spu_id = 0;
                    item.attribute_name = "类目关键词";
                    item.is_optional = false;
                    v.attribute_posting.push_back(item);
                }
            }
            for(AttributeMap::iterator it = attribute_map_.begin(); it!=attribute_map_.end(); ++it)
            {
                it->second.Compute(category_list_);
            }
            //for(std::size_t i=1;i<category_list_.size();i++)
            //{
            //    const Category& c = category_list_[i];
            //}
        }
        std::string resource_dir = dir+"/resource";
        if(boost::filesystem::exists(resource_dir))
        {
            LOG(INFO)<<resource_dir<<" exists"<<std::endl;
            if(Load(resource_dir))
            {
                LOG(INFO)<<"load "<<resource_dir<<" finish"<<std::endl;
                std::vector<std::string> queries;
                //queries.push_back("iphone");
                //queries.push_back("iphone手机");
                //queries.push_back("iphone4");
                //queries.push_back("iphone4手机");
                //queries.push_back("手机iphone4");
                //queries.push_back("苹果iphone4");
                //queries.push_back("苹果iphone4手机特价销售 16G");
                //queries.push_back("苹果手机壳");
                //queries.push_back("大金空调");
                //queries.push_back("性感连衣裙");
                //queries.push_back("长款T恤");

                for(std::size_t i=0;i<queries.size();i++)
                {
                    const std::string& query = queries[i];
                    std::cerr<<"[QUERY]"<<query<<std::endl;
                    Document doc;
                    doc.property("Title") = str_to_propstr(query);
                    std::string category;
                    Evaluate(doc, category);
                    std::cerr<<"[CATEGORY]"<<category<<std::endl;
                }
                return true;
            }
            else
            {
                return false;
            }
        }
        boost::filesystem::create_directories(resource_dir);
        std::string work_dir = resource_dir+"/work_dir";
        B5MHelper::PrepareEmptyDir(work_dir);
        ngram_processor_ = new NgramProcessor(work_dir);
        std::string offer_scd = dir+"/OFFER.SCD";
        {
            ScdDocProcessor::ProcessorType p = boost::bind(&ProductClassifier::OfferInsert_, this, _1);
            ScdDocProcessor sd_processor(p, 20);
            sd_processor.AddInput(offer_scd);
            sd_processor.Process();
            ngram_processor_->Finish(boost::bind(&ProductClassifier::NgramProcess_, this, _1));
        }
        delete ngram_processor_;
        return true;
    }
    bool MaxentTrain(const std::string& dir)
    {
        std::string category_file = dir+"/category";
        LoadCategoryInfo_(category_file);
        std::string resource_dir = dir+"/resource";
        if(boost::filesystem::exists(resource_dir))
        {
            if(!Load(resource_dir)) return false;
        }
        else return false;
        std::string work_dir = resource_dir+"/work_dir";
        std::string maxent_dir = work_dir+"/maxent";
        if(!boost::filesystem::exists(maxent_dir))
        {
            boost::filesystem::create_directories(maxent_dir);
        }
        maxent_ofs_list_.resize(category_list_.size(), NULL);
        for(std::size_t i=0;i<maxent_ofs_list_.size();i++)
        {
            const Category& c = category_list_[i];
            if(c.is_parent)
            {
                std::string file = maxent_dir+"/"+boost::lexical_cast<std::string>(i)+".meinput";
                maxent_ofs_list_[i] = new std::ofstream(file.c_str());
            }
        }
        std::string offer_scd = dir+"/OFFER.SCD";
        ScdDocProcessor::ProcessorType p = boost::bind(&ProductClassifier::OfferMaxent_, this, _1);
        ScdDocProcessor sd_processor(p, 5);
        sd_processor.AddInput(offer_scd);
        sd_processor.Process();
        for(std::size_t i=0;i<maxent_ofs_list_.size();i++)
        {
            if(maxent_ofs_list_[i]!=NULL)
            {
                maxent_ofs_list_[i]->close();
            }
        }
        return true;
    }
    //train in second stage
    bool JointTrain(const std::string& dir)
    {
        std::string category_file = dir+"/category";
        LoadCategoryInfo_(category_file);
        std::string resource_dir = dir+"/resource";
        std::string work_dir = resource_dir+"/work_dir";
        std::string mid_file = work_dir+"/mid";
        std::string cid_file= work_dir+"/cid";
        std::string spu_scd = dir+"/SPU.SCD";
        std::string offer_scd = dir+"/OFFER.SCD";
        if(boost::filesystem::exists(cid_file))
        {
            Reader reader(cid_file);
            reader.Open();
            std::pair<term_t, std::size_t> v;
            while(reader.Next(v))
            {
                category_list_[v.first].offer_count = v.second;
            }
            reader.Close();
            for(std::size_t i=1;i<category_list_.size();i++)
            {
                Category* c = &(category_list_[i]);
                std::size_t count = c->offer_count;
                while(true)
                {
                    Category* pc = &(category_list_[c->parent_cid]);
                    pc->offer_count += count;
                    c = pc;
                    if(c->cid==0) break;
                }
            }
        }
        else return false;
        std::string joint_path = work_dir+"/joint";
        std::string file2 = joint_path+"/file2";
        std::string joint_mid_file = joint_path+"/mid";
        if(boost::filesystem::exists(joint_mid_file))
        {
            {
                //joint_dic_ = new JointDic;
                //NgramProcessor::Reader reader(joint_mid_file);
                //reader.Open();
                //std::cerr<<"TOTAL COUNT: "<<reader.Count()<<std::endl;
                //B5mThreadPool<Ngram> pool(5, boost::bind(&ProductClassifier::TryJointDicInsert_, this, _1));
                //Ngram* ngram = new Ngram;
                //std::size_t p=0;
                //dic_size_ = 0;
                //while(reader.Next( (*ngram) ))
                //{
                //    ++p;
                //    if(p%100000==0)
                //    {
                //        std::cerr<<"joint mid loading "<<p<<","<<dic_size_<<std::endl;
                //    }
                //    pool.schedule(ngram);
                //    ngram = new Ngram;
                //}
                //reader.Close();
                //pool.wait();
            }
            {
                dic_size_ = 0;
                NgramProcessor::Reader reader(mid_file);
                reader.Open();
                dic_ = new NgramDic;
                std::cerr<<"TOTAL COUNT: "<<reader.Count()<<std::endl;
                B5mThreadPool<Ngram> pool(5, boost::bind(&ProductClassifier::TryDicInsertDirectly_, this, _1));
                Ngram* ngram = new Ngram;
                std::size_t p=0;
                while(reader.Next( (*ngram) ))
                {
                    ++p;
                    if(p%100000==0)
                    {
                        std::cerr<<"mid loading "<<p<<","<<dic_size_<<std::endl;
                    }
                    pool.schedule(ngram);
                    ngram = new Ngram;
                }
                reader.Close();
                pool.wait();
            }
            Test(offer_scd, 5);
            return true;
        }
        else if(boost::filesystem::exists(file2))
        {
            //if(boost::filesystem::exists(mid_file))
            //{
            //    boost::filesystem::remove_all(mid_file);
            //}
            ngram_processor_ = new NgramProcessor(joint_path);
            ngram_processor_->SetMaxContextProp(1.0);//never ignore any ngram
            ngram_processor_->Finish(boost::bind(&ProductClassifier::JointNgramProcess_, this, _1));
            delete ngram_processor_;
            return true;
        }
        if(boost::filesystem::exists(mid_file))
        {
            //ngram_processor_->ApplyCategory(category_list_);
            NgramProcessor::Reader reader(mid_file);
            reader.Open();
            std::cerr<<"TOTAL COUNT: "<<reader.Count()<<std::endl;
            B5mThreadPool<Ngram> pool(5, boost::bind(&ProductClassifier::TryDicInsert2_, this, _1));
            Ngram* ngram = new Ngram;
            std::size_t p=0;
            dic_ = new NgramDic;
            while(reader.Next( (*ngram) ))
            {
                ++p;
                if(p%100000==0)
                {
                    std::cerr<<"mid loading "<<p<<","<<dic_size_<<std::endl;
                }
                pool.schedule(ngram);
                ngram = new Ngram;
            }
            reader.Close();
            pool.wait();
        }
        else return false;
        ngram_processor_ = new NgramProcessor(joint_path);
        ngram_processor_->SetMaxContextProp(1.0);//never ignore any ngram
        ScdDocProcessor::ProcessorType p = boost::bind(&ProductClassifier::OfferTrain2_, this, _1);
        ScdDocProcessor sd_processor(p, 5);
        sd_processor.AddInput(offer_scd);
        sd_processor.Process();
        ngram_processor_->Finish(boost::bind(&ProductClassifier::JointNgramProcess_, this, _1));
        delete ngram_processor_;
        return true;
    }
    void Test(const std::string& scd, int thread_num = 1)
    {
        if(dic_->Size()>0)
        {
            ScdDocProcessor::ProcessorType p = boost::bind(&ProductClassifier::Test_, this, _1);
            ScdDocProcessor sd_processor(p, thread_num);
            sd_processor.AddInput(scd);
            sd_processor.Process();
            for(std::size_t i=1;i<=3;i++)
            {
                double v = (double)recall_[i]->first/recall_[i]->second;
                std::cerr<<"[STAT-RECALL]("<<i<<")"<<recall_[i]->first<<","<<recall_[i]->second<<","<<v<<std::endl;
            }
            for(std::size_t i=1;i<=3;i++)
            {
                double vaccuracy = (double)accuracy_[i]->first/accuracy_[i]->second;
                std::cerr<<"[STAT-ACCURACY]("<<i<<")"<<accuracy_[i]->first<<","<<accuracy_[i]->second<<","<<vaccuracy<<std::endl;
            }
            double average_time = time_/recall_[1]->second;
            double qps = (double)recall_[1]->second/time_;
            std::cerr<<"[STAT-TIME]"<<time_<<","<<average_time<<","<<qps<<std::endl;
            std::vector<std::pair<std::size_t, term_t> > error_list;
            for(CategoryCount::const_iterator it = error_.begin();it!=error_.end();++it)
            {
                error_list.push_back(std::make_pair(it->second, it->first));
            }
            std::sort(error_list.begin(), error_list.end());
            for(std::size_t i=0;i<error_list.size();i++)
            {
                std::cerr<<"[ERROR]"<<category_list_[error_list[i].second].name<<","<<error_list[i].first<<std::endl;
            }
        }
    }
    void Evaluate(const Document& doc, std::string& category)
    {
        //std::string attribute;
        //doc.getString("Attribute", attribute);
        //if(!attribute.empty())
        //{
        //    std::vector<Attribute> attributes;
        //    ProductMatcher::ParseAttributes(UString(attribute, UString::UTF_8), attributes);
        //    for(std::size_t i=0;i<attributes.size();i++)
        //    {
        //        boost::algorithm::to_lower(attributes[i].name);
        //        if(attributes[i].name=="isbn")
        //        {
        //            category = book_category_;
        //            return;
        //        }
        //    }
        //}
        int64_t level = 2;
        doc.getProperty("Level", level);
        if(level>3) level = 3;
        else if(level<1) level = 1;
        //use top to bottom method only
        int64_t method = 1;
        doc.getProperty("Method", method);
        term_t pcid = 0;//parent cid
        for(int64_t ilevel=1;ilevel<=level;ilevel++)
        {
            term_t cid = 0;
            if(method==2)
            {
                term_t cid1 = 0;
                {
                    AnalyzeItem item;
                    std::bitset<3> param;
                    param[0] = true;
                    param[1] = false;
                    param[2] = true;
                    Analyze_(doc, param, item);
                    double dscore = 0.0;
                    Evaluate_(item, pcid, cid1, dscore);
                }
                term_t cid2 = 0;
                {
                    AnalyzeItem item;
                    std::bitset<3> param;
                    param[0] = false;
                    param[1] = true;
                    param[2] = true;
                    Analyze_(doc, param, item);
                    double dscore = 0.0;
                    Evaluate_(item, pcid, cid2, dscore);
                }
                if(cid1!=0&&cid2!=0)
                {
                    if(cid1==cid2)
                    {
                        cid = cid1;
                    }
                }
                else if(cid1>0)
                {
                    cid = cid1;
                }
                else if(cid2>0)
                {
                    cid = cid2;
                }
            }
            else
            {
                AnalyzeItem item;
                std::bitset<3> param;
                param[0] = true;
                param[1] = true;
                param[2] = true;
                Analyze_(doc, param, item);
                double dscore = 0.0;
                Evaluate_(item, pcid, cid, dscore);
            }
            if(cid>0)
            {
                category = category_list_[cid].name;
                //if(ilevel==1&&cid==book_cid_) break;
                pcid = cid;
            }
            else break;
        }
        //std::string ocategory;
        //doc.getString("Category", ocategory);
        //if(IsMatch_(ocategory, category))
        //{
        //    std::cerr<<"[TRUE]"<<dscore<<std::endl;
        //}
        //else
        //{
        //    std::cerr<<"[FALSE]"<<dscore<<std::endl;
        //}
        //if(level==1) return;
        //pcid = cid;
        //dscore = 0.0;
        //r = Evaluate_(item, pcid, cid, dscore);
        //if(r)
        //{
        //    category = category_list_[cid].name;
        //}
    }

private:

    void GetCategoryIdList_(const std::string& category, std::vector<term_t>& cid_list)
    {
        std::vector<term_t> rlist;
        term_t cid=0;
        if(!GetCategoryId_(category, cid)) return;
        while(cid!=0)
        {
            rlist.push_back(cid);
            cid = category_list_[cid].parent_cid;
        }
        cid_list.assign(rlist.rbegin(), rlist.rend());
    }

    bool IsMatch_(const std::string& category, const std::string& tcategory)
    {
        std::vector<term_t> cid_list;
        GetCategoryIdList_(category, cid_list);
        if(cid_list.empty()) return false;
        std::vector<term_t> tcid_list;
        GetCategoryIdList_(tcategory, tcid_list);
        if(tcid_list.empty()) return false;
        return boost::algorithm::starts_with(cid_list, tcid_list) || boost::algorithm::starts_with(tcid_list, cid_list);
    }
    term_t GetTerm_(const std::string& str)
    {
        term_t term = izenelib::util::HashFunction<std::string>::generateHash32(str);
        return term;
    }
    void GetWord_(const std::string& text, word_t& word)
    {
        UString title(text, UString::UTF_8);
        std::vector<idmlib::util::IDMTerm> terms;
        analyzer_->GetTermList(title, terms);
        for(uint32_t i=0;i<terms.size();i++)
        {
            std::string str = terms[i].TextString();
            if(terms[i].tag==idmlib::util::IDMTermTag::SYMBOL)
            {
                continue;
            }
            boost::algorithm::to_lower(str);
            word.push_back(GetTerm_(str));
        }
    }
    void GetSentence_(const std::string& text, Sentence& sentence)
    {
        UString title(text, UString::UTF_8);
        std::vector<idmlib::util::IDMTerm> terms;
        analyzer_->GetTermList(title, terms);
        Word word;
        for(uint32_t i=0;i<terms.size();i++)
        {
            std::string str = terms[i].TextString();
            if(terms[i].tag==idmlib::util::IDMTermTag::SYMBOL)
            {
                if(str!="-")
                {
                    continue;
                }
            }
            boost::algorithm::to_lower(str);
            word.word.push_back(GetTerm_(str));
            word.text.push_back(str);
        }
        sentence.push_back(word);
    }
    void GetOCSentence_(const std::string& oc, Sentence& sentence)
    {
        std::vector<std::string> texts;
        boost::algorithm::split(texts, oc, boost::algorithm::is_any_of(">"));
        for(std::size_t t=0;t<texts.size();t++)
        {
            const std::string& text = texts[t];
            UString title(text, UString::UTF_8);
            std::vector<idmlib::util::IDMTerm> terms;
            analyzer_->GetTermList(title, terms);
            Word word;
            for(uint32_t i=0;i<terms.size();i++)
            {
                std::string str = terms[i].TextString();
                if(terms[i].tag==idmlib::util::IDMTermTag::SYMBOL)
                {
                    if(str!="-")
                    {
                        continue;
                    }
                }
                boost::algorithm::to_lower(str);
                word.word.push_back(GetTerm_(str));
                word.text.push_back(str);
            }
            sentence.push_back(word);
        }
    }
    void AttributeBoosting_(const word_t& key, double weight, CategoryScore& boosting)
    {
        word_t found;
        word_t frag;
        std::size_t pos=0;
        std::size_t last_found_pos = 0;
        std::vector<const AttributeMapValue*> values;
        const AttributeMapValue* foundv = NULL;
        while(true)
        {
            if(pos>=key.size()) break;
            frag.push_back(key[pos]);
            int status = -1;
            AttributeMap::const_iterator it = attribute_map_.lower_bound(frag);
            if(it==attribute_map_.end()) status = -1;
            else
            {
                if(boost::algorithm::starts_with(it->first, frag))
                {
                    if(it->first==frag) status = 1;
                    else status = 0;
                }
                else status = -1;
            }
            if(status==-1)
            {
                if(!found.empty())
                {
                    values.push_back(foundv);
                    found.resize(0);
                    pos = last_found_pos+1;
                }
                else
                {
                    ++pos;
                }
                frag.resize(0);
            }
            else if(status==0)
            {
                ++pos;
            }
            else
            {
                found = frag;
                foundv = &(it->second);
                //all mode
                values.push_back(foundv);
                found.clear();

                last_found_pos = pos;
                ++pos;
            }
        }
        if(!found.empty())
        {
            values.push_back(foundv);
            found.resize(0);
        }

        double base = 1.0;
        double binc = 0.1;
        double max = 2.0;
        if(weight>=0.8) max = 100.0;
        for(std::size_t i=0;i<values.size();i++)
        {
            const std::vector<AttributePostingItem>& posting = values[i]->attribute_posting;
            for(std::size_t a=0;a<posting.size();a++)
            {
                if(posting[a].is_optional) continue;
                double inc = binc;
                if(posting[a].attribute_name=="型号") 
                {
                    inc = 0.5;
                    if(weight>=0.8)
                    {
                        inc*=50;
                    }
                }
                std::vector<term_t> cid_list;
                term_t cid = posting[a].cid;
                while(true)
                {
                    if(cid==0) break;
                    cid_list.push_back(cid);
                    cid = category_list_[cid].parent_cid;
                }
                for(std::size_t c=0;c<cid_list.size();c++)
                {
                    term_t cid = cid_list[c];
                    CategoryScore::iterator bit = boosting.find(cid);
                    if(bit==boosting.end())
                    {
                        boosting.insert(std::make_pair(cid, base+inc));
                    }
                    else
                    {
                        bit->second+=inc;
                        if(bit->second>max) bit->second = max;
                    }
                }
            }
        }
    }
    void Analyze_(const Document& doc, const std::bitset<3>& param, AnalyzeItem& item)
    {
        item.doc = doc;
        if(param[0])
        {
            std::string query;
            doc.getString("Title", query);
            word_t word;
            GetWord_(query, word);
            dic_->Search(word, 1, item.tvalues);
        }
        if(param[1])
        {
            std::string oc;
            doc.getString("OriginalCategory", oc);
            Sentence sentence;
            GetOCSentence_(oc, sentence);
            for(std::size_t i=0;i<sentence.size();i++)
            {
                const Word& word = sentence[i];
                std::vector<NgramValue> values;
                dic_->Search(word.word, 2, values);
                //ngram_processor_->DicTest(word.word, 2, values);
                item.ocvalues.insert(item.ocvalues.end(), values.begin(), values.end());
                //std::vector<NgramValue> tvalues;
                //ngram_processor_->DicTest(word.word, 1, tvalues);
                //item.tvalues.insert(item.tvalues.end(), tvalues.begin(), tvalues.end());
            }
        }
        if(param[2])
        {
            item.price = ProductPrice::ParseDocPrice(doc, "Price");
        }
        //std::cerr<<"[A]"<<item.tvalues.size()<<","<<item.ocvalues.size()<<std::endl;
        
    }
    void Calculate_(const AnalyzeItem& item, NgramValue& nv, term_t pcid, CategoryScore& cscore)
    {
        //std::cerr<<"[N-WORD]";
        //for(std::size_t j=0;j<nv.word.size();j++)
        //{
        //    std::cerr<<nv.word[j]<<",";
        //}
        //std::cerr<<std::endl;
        //NgramProcessor::CalculateCategoryProb(nv, category_list_, pcid);
        //CategoryScore cid_score;
        const Category& pc = category_list_[pcid];
        const std::vector<term_t>& target_cids = pc.children;
        //for(std::size_t j=0;j<target_cids.size();j++)
        //{
        //    const Category& c = category_list_[target_cids[j]];
        //    cid_score[c.cid] = 0.0;
        //}
        //double prop_sum = 0.0;
        //for(std::size_t j=0;j<target_cids.size();j++)
        //{
        //    term_t cid = target_cids[j];
        //    NgramValue::const_iterator it = nv.find(cid);
        //    if(it==nv.end()) continue;
        //    const NCI& nci = it->second;
        //    double score = nci.prob;
        //    std::size_t count = nci.freq;
        //    const Category& c = category_list_[cid];
        //    double prop = (double)count/c.offer_count;
        //    std::cerr<<"[CO]"<<c.name<<","<<score<<","<<prop<<std::endl;
        //    cid_score[cid] = prop;
        //    prop_sum += prop;
        //}
        //double en = 0.0;
        //for(CategoryScore::iterator it = cid_score.begin();it!=cid_score.end();++it)
        //{
        //    it->second /= prop_sum;
        //    double p = it->second;
        //    if(p>0.0)
        //    {
        //        en += p*std::log(p)/std::log(2.0);
        //    }
        //}
        //en *= -1.0;
        //std::cerr<<"[COEN]"<<en<<std::endl;
        CategoryScore boosting;
        if(nv.type==1)
        {
            //AttributeBoosting_(nv.word, 0.25, boosting);
        }
        CategoryScore inner;
        for(std::size_t j=0;j<target_cids.size();j++)
        {
            term_t cid = target_cids[j];
            NgramValue::const_iterator it = nv.find(cid);
            if(it==nv.end()) continue;
            const NCI& nci = it->second;
            double score = nci.prob;
            //std::size_t count = nci.freq;
            CategoryScore::iterator bit = boosting.find(cid);
            if(bit!=boosting.end())
            {
                score*=bit->second;
            }
            if(item.price>0.0)
            {
                if(nci.price==0.0) score*=0.1;
                else
                {
                    double pratio = std::min(item.price, nci.price)/std::max(item.price, nci.price);
                    score *= pratio;
                }
            }
            //const Category& c = category_list_[cid];
            //if(boost::algorithm::starts_with(c.name, book_category_)) score*=0.5;
            score*=nv.weight;
            inner[cid] = score;
            //std::cerr<<"[DEBUG]"<<category_list_[cid].name<<","<<score<<","<<count<<","<<category_list_[cid].offer_count<<std::endl;
        }
        for(CategoryScore::iterator it=inner.begin();it!=inner.end();++it)
        {
            CategoryScore::iterator iit = cscore.find(it->first);
            if(iit==cscore.end())
            {
                cscore.insert(std::make_pair(it->first, it->second));
            }
            else
            {
                iit->second+=it->second;
            }
        }
    }
    void Calculate2_(const AnalyzeItem& item, NgramValue& nv, CategoryScore& cscore)
    {
        //std::cerr<<"[N-WORD]";
        //for(std::size_t j=0;j<nv.word.size();j++)
        //{
        //    std::cerr<<nv.word[j]<<",";
        //}
        //std::cerr<<std::endl;
        //NgramProcessor::CalculateCategoryProb(nv, category_list_, pcid);
        //CategoryScore cid_score;
        //for(std::size_t i=1;i<category_list_.size();i++)
        //{
        //    const Category& c = category_list_[i];
        //    cid_score[c.cid] = 0.0;
        //}
        //double prop_sum = 0.0;
        //for(std::size_t i=1;i<category_list_.size();i++)
        //{
        //    const Category& c = category_list_[i];
        //    if(c.depth!=2) continue;
        //    term_t cid = c.cid;
        //    NgramValue::const_iterator it = nv.find(cid);
        //    if(it==nv.end()) continue;
        //    const NCI& nci = it->second;
        //    double score = nci.prob;
        //    std::size_t count = nci.freq;
        //    double prop = (double)count/c.offer_count;
        //    std::cerr<<"[CO]"<<c.name<<","<<score<<","<<prop<<std::endl;
        //    cid_score[cid] = score;
        //    prop_sum += prop;
        //}
        //double en = 0.0;
        //for(CategoryScore::iterator it = cid_score.begin();it!=cid_score.end();++it)
        //{
        //    it->second /= prop_sum;
        //    double p = it->second;
        //    if(p>0.0)
        //    {
        //        en += p*std::log(p)/std::log(2.0);
        //    }
        //}
        //en *= -1.0;
        //std::cerr<<"[COEN]"<<en<<std::endl;
        CategoryScore boosting;
        if(nv.type==1)
        {
            //AttributeBoosting_(nv.word, 0.25, boosting);
        }
        CategoryScore inner;
        for(std::size_t i=1;i<category_list_.size();i++)
        {
            const Category& c = category_list_[i];
            term_t cid = c.cid;
            if(c.depth!=2) continue;
            NgramValue::const_iterator it = nv.find(c.cid);
            if(it==nv.end()) continue;
            const NCI& nci = it->second;
            double score = nci.prob;
            //std::size_t count = nci.freq;
            //if(boost::algorithm::starts_with(c.name, book_category_)) continue;
            CategoryScore::iterator bit = boosting.find(cid);
            if(bit!=boosting.end())
            {
                score*=bit->second;
            }
            if(item.price>0.0)
            {
                if(nci.price==0.0) score*=0.1;
                else
                {
                    double pratio = std::min(item.price, nci.price)/std::max(item.price, nci.price);
                    score *= pratio;
                }
            }
            //if(boost::algorithm::starts_with(c.name, book_category_)) score*=0.5;
            score*=nv.weight;
            inner[cid] = score;
            //std::cerr<<"[DEBUG]"<<category_list_[cid].name<<","<<score<<","<<count<<","<<category_list_[cid].offer_count<<std::endl;
        }
        for(CategoryScore::iterator it=inner.begin();it!=inner.end();++it)
        {
            CategoryScore::iterator iit = cscore.find(it->first);
            if(iit==cscore.end())
            {
                cscore.insert(std::make_pair(it->first, it->second));
            }
            else
            {
                iit->second+=it->second;
            }
        }
    }
    bool Evaluate_(AnalyzeItem& item, term_t pcid, term_t& cid, double& dscore)
    {

        //std::cerr<<"[Q-WORD]";
        //for(std::size_t i=0;i<word.size();i++)
        //{
        //    std::cerr<<word[i]<<",";
        //}
        //std::cerr<<std::endl;
        CategoryScore cscore;
        for(std::size_t i=0;i<item.tvalues.size();i++)
        {
            NgramValue& nv = item.tvalues[i];
            Calculate_(item, nv, pcid, cscore);
        }
        for(std::size_t i=0;i<item.ocvalues.size();i++)
        {
            NgramValue& nv = item.ocvalues[i];
            Calculate_(item, nv, pcid, cscore);
        }
        if(!me_classifiers_.empty()&& (!item.tvalues.empty()||!item.ocvalues.empty()))
        {
            MaxentClassifier* mc = me_classifiers_[pcid];
            //mc = NULL;
            if(mc!=NULL)
            {
                std::vector<term_t> ids(item.tvalues.size());
                ids.reserve(item.tvalues.size()+item.ocvalues.size());
                for(std::size_t i=0;i<item.tvalues.size();i++)
                {
                    term_t id = GenNVID_(item.tvalues[i].word, false);
                    ids.push_back(id);
                }
                for(std::size_t i=0;i<item.ocvalues.size();i++)
                {
                    term_t id = GenNVID_(item.ocvalues[i].word, true);
                    ids.push_back(id);
                }
                CategoryScore me_score;
                mc->Evaluate(ids, me_score);
                for(CategoryScore::iterator it=cscore.begin();it!=cscore.end();++it)
                {
                    double mes = 0.0;
                    CategoryScore::const_iterator meit = me_score.find(it->first);
                    if(meit!=me_score.end()) mes = meit->second;
                    //it->second *= mes*mes;
                    it->second *= mes;
                }
            }
        }
        std::pair<double, term_t> max(0.0, 0);
        for(CategoryScore::iterator it=cscore.begin();it!=cscore.end();++it)
        {
            if(it->second>max.first)
            {
                max.first = it->second;
                max.second = it->first;
            }
        }
        if(max.first>0.0)
        {
            cid = max.second;
            dscore = max.first;
            return true;
        }
        return false;
    }
    bool Evaluate2_(AnalyzeItem& item, term_t& cid)
    {
        //std::cerr<<"[Q-WORD]";
        //for(std::size_t i=0;i<word.size();i++)
        //{
        //    std::cerr<<word[i]<<",";
        //}
        //std::cerr<<std::endl;
        CategoryScore cscore;
        for(std::size_t i=0;i<item.tvalues.size();i++)
        {
            NgramValue& nv = item.tvalues[i];
            Calculate2_(item, nv, cscore);
        }
        for(std::size_t i=0;i<item.ocvalues.size();i++)
        {
            NgramValue& nv = item.ocvalues[i];
            Calculate2_(item, nv, cscore);
        }
        std::pair<double, term_t> max(0.0, 0);
        for(CategoryScore::iterator it=cscore.begin();it!=cscore.end();++it)
        {
            if(it->second>max.first)
            {
                max.first = it->second;
                max.second = it->first;
            }
        }
        if(max.first>0.0)
        {
            cid = max.second;
            return true;
        }
        return false;
    }
    void LoadCategoryInfo_(const std::string& file)
    {
        std::string line;
        std::ifstream ifs(file.c_str());
        category_list_.resize(1);
        category_list_[0].is_parent = true;
        category_list_[0].depth = 0;
        category_list_[0].cid = 0;
        while(getline(ifs, line))
        {
            boost::algorithm::trim(line);
            std::vector<std::string> vec;
            boost::algorithm::split(vec, line, boost::algorithm::is_any_of(","));
            if(vec.size()<1) continue;
            const std::string& scategory = vec[0];
            if(scategory.empty()) continue;
            term_t cid = category_list_.size();
            Category c;
            c.name = scategory;
            c.cid = cid;
            c.parent_cid = 0;
            c.is_parent = false;
            c.has_spu = false;
            c.offer_count = 0;
            std::set<std::string> akeywords;
            std::set<std::string> rkeywords;
            for(uint32_t i=1;i<vec.size();i++)
            {
                std::string keyword = vec[i];
                bool remove = false;
                if(keyword.empty()) continue;
                if(keyword[0]=='+')
                {
                    keyword = keyword.substr(1);
                }
                else if(keyword[0]=='-')
                {
                    keyword = keyword.substr(1);
                    remove = true;
                }
                if(keyword.empty()) continue;
                if(!remove)
                {
                    akeywords.insert(keyword);
                }
                else
                {
                    rkeywords.insert(keyword);
                }
            }
            std::vector<std::string> cs_list;
            boost::algorithm::split( cs_list, c.name, boost::algorithm::is_any_of(">") );
            c.depth=cs_list.size();
            std::vector<std::string> keywords_vec;
            boost::algorithm::split( keywords_vec, cs_list.back(), boost::algorithm::is_any_of("/") );
            for(uint32_t i=0;i<keywords_vec.size();i++)
            {
                akeywords.insert(keywords_vec[i]);
            }
            for(std::set<std::string>::const_iterator it = rkeywords.begin();it!=rkeywords.end();it++)
            {
                akeywords.erase(*it);
            }
            for(std::set<std::string>::const_iterator it = akeywords.begin();it!=akeywords.end();it++)
            {
                UString uc(*it, UString::UTF_8);
                if(uc.length()<=1) continue;
                c.keywords.push_back(*it);
            }
            if(c.depth>1)
            {
                std::string parent_name;
                for(uint32_t i=0;i<c.depth-1;i++)
                {
                    if(!parent_name.empty())
                    {
                        parent_name+=">";
                    }
                    parent_name+=cs_list[i];
                }
                c.parent_cid = category_index_[parent_name];
                //std::cerr<<"cid "<<c.cid<<std::endl;
                //std::cerr<<"parent cid "<<c.parent_cid<<std::endl;
            }
            else
            {
                c.parent_cid = 0;
            }
            Category& pc = category_list_[c.parent_cid];
            pc.is_parent = true;
            pc.children.push_back(cid);
            category_index_[scategory] = cid;
            category_list_.push_back(c);
        }
        ifs.close();
        book_category_ = "图书音像";
        book_cid_ = category_index_[book_category_];
        std::cerr<<"book_cid : "<<book_cid_<<std::endl;
    }
    void SpuInsert_(ScdDocument& doc)
    {
        spu_list_.push_back(doc);
        uint32_t spu_id = spu_list_.size()-1;
        std::string category;
        std::string title;
        std::string attribute;
        doc.getString("Category", category);
        doc.getString("Title", title);
        doc.getString("Attribute", attribute);
        term_t cid=0;
        if(!GetCategoryId_(category, cid)) return;
        if(category.find("配件")!=std::string::npos) return;
        UString uattribute(attribute, UString::UTF_8);
        std::vector<Attribute> attributes;
        ProductMatcher::ParseAttributes(uattribute, attributes);
        for (size_t i = 0; i < attributes.size(); ++i)
        {
            const std::string& name = attributes[i].name;
            for(std::size_t j=0;j<attributes[i].values.size();j++)
            {
                const std::string& value = attributes[i].values[j];
                word_t word;
                GetWord_(value, word);
                AttributeMapValue& v = attribute_map_[word];
                v.name = value;
                AttributePostingItem item;
                item.cid = cid;
                item.spu_id = spu_id;
                item.attribute_name = name;
                item.is_optional = attributes[i].is_optional;
                v.attribute_posting.push_back(item);
            }
        }

        //for (size_t i = 0; i < attributes.size(); ++i)
        //    if (product.attributes[i].name == "品牌")
        //    {
        //        if (product.attributes[i].values.empty() || product.attributes[i].values.size() < 2) continue;

        //        size_t count = 0;
        //        size_t tmp_id = 0;
        //        string lower_string;
        //        for (size_t j = 0; j < product.attributes[i].values.size(); ++j)
        //        {
        //            lower_string = product.attributes[i].values[j];
        //            boost::to_lower(lower_string);
        //            if (synonym_map_.find(lower_string)!=synonym_map_.end())//term is not in synonym map
        //            {
        //                ++count;
        //                tmp_id = synonym_map_[lower_string];
        //            }
        //        }
        //        if (!count)//new synonym set
        //        {
        //            string st;
        //            for (size_t j = 0; j < product.attributes[i].values.size(); ++j)
        //            {
        //                lower_string = product.attributes[i].values[j];
        //                boost::to_lower(lower_string);
        //                synonym_map_.insert(std::make_pair(lower_string, synonym_dict_size));
        //                synonym_pairs.push_back(std::make_pair(synonym_dict_size, lower_string));
        //                st += (lower_string + '/');
        //            }
        //            term_set.push_back(st);
        //            ++synonym_dict_size;
        //        }
        //        else if (count < product.attributes[i].values.size())//add term in synonym set
        //        {
        //            for (size_t j = 0; j < product.attributes[i].values.size(); ++j)
        //            {
        //                lower_string = product.attributes[i].values[j];
        //                boost::to_lower(lower_string);
        //                if (synonym_map_.find(lower_string) == synonym_map_.end())
        //                {
        //                    synonym_map_.insert(std::make_pair(lower_string, tmp_id));
        //                    synonym_pairs.push_back(std::make_pair(tmp_id, lower_string));
        //                    term_set[tmp_id] += lower_string + '/';
        //                }
        //            }
        //        }
        //    }
    }
    void OfferInsert_(ScdDocument& doc)
    {
        std::string category;
        std::string title;
        doc.getString("Category", category);
        doc.getString("Title", title);
        //if(!boost::algorithm::starts_with(category, "手机数码"))
        //{
        //    return;
        //}
        term_t cid=0;
        if(!GetCategoryId_(category, cid)) return;
        ngram_processor_->IncCidCount(cid);
        double price = ProductPrice::ParseDocPrice(doc, "Price");
        term_t oid = oid_.fetch_add(1);
        std::vector<Ngram> ngram_list;
        {
            Sentence sentence;
            GetSentence_(title, sentence);
            for(std::size_t i=0;i<sentence.size();i++)
            {
                const Word& word = sentence[i];
                Ngram ngram;
                ngram.word = word.word;
                ngram.text = word.text;
                ngram.type = 1;
                ngram.freq = 1;
                ngram.oid = oid;
                NCI nci;
                nci.cid = cid;
                nci.freq = 1;
                nci.price = price;
                ngram.nci_array.push_back(nci);
                ngram_list.push_back(ngram);
            }
        }
        std::string oc;
        doc.getString("OriginalCategory", oc);
        if(!oc.empty())
        {
            Sentence sentence;
            GetOCSentence_(oc, sentence);
            for(std::size_t i=0;i<sentence.size();i++)
            {
                const Word& word = sentence[i];
                Ngram ngram;
                ngram.word = word.word;
                ngram.text = word.text;
                //std::cerr<<"[OCTEXT]"<<ngram.GetText()<<std::endl;
                ngram.type = 2;
                ngram.freq = 1;
                ngram.oid = oid;
                NCI nci;
                nci.cid = cid;
                nci.freq = 1;
                nci.price = price;
                ngram.nci_array.push_back(nci);
                ngram_list.push_back(ngram);
            }
        }
        ngram_processor_->Insert(ngram_list);
    }
    term_t GenNVID_(const word_t& word, bool is_oc) const
    {
        word_t key;
        if(is_oc) key.push_back(0);
        key.insert(key.end(), word.begin(), word.end());
        return hasher_(key);
    }
    void OfferTrain2_(ScdDocument& doc)
    {
        AnalyzeItem item;
        std::bitset<3> param;
        param[0] = true;
        param[1] = false;
        param[2] = false;
        Analyze_(doc, param, item);
        word_t ids;
        for(std::size_t i=0;i<item.tvalues.size();i++)
        {
            term_t id = GenNVID_(item.tvalues[i].word, false);
            ids.push_back(id);
            //std::cerr<<"[ID]"<<ids.back()<<std::endl;
        }
        //std::cerr<<std::endl;
        if(ids.size()<2) return;
        std::sort(ids.begin(), ids.end());
        std::string category;
        std::string title;
        doc.getString("Category", category);
        doc.getString("Title", title);
        term_t cid=0;
        if(!GetCategoryId_(category, cid)) return;
        double price = ProductPrice::ParseDocPrice(doc, "Price");
        term_t oid = oid_.fetch_add(1);
        Ngram ngram;
        ngram.word = ids;
        ngram.type = 1;
        ngram.freq = 1;
        ngram.oid = oid;
        NCI nci;
        nci.cid = cid;
        nci.freq = 1;
        nci.price = price;
        ngram.nci_array.push_back(nci);
        std::vector<Ngram> ngram_list(1, ngram);
        ngram_processor_->Insert(ngram_list);
    }

    void OfferMaxent_(ScdDocument& doc)
    {
        static boost::mt19937 gen;
        boost::uniform_int<term_t> dist(1, 100);
        term_t rv = dist(gen);
        if(rv>5) return;
        std::string category;
        std::string title;
        std::string original_category;
        doc.getString("Category", category);
        term_t cid=0;
        if(!GetCategoryId_(category, cid)) return;
        std::vector<term_t> cid_list;
        while(true)
        {
            cid_list.push_back(cid);
            if(cid==0) break;
            cid = category_list_[cid].parent_cid;
        }
        std::reverse(cid_list.begin(), cid_list.end());
        AnalyzeItem item;
        std::bitset<3> param;
        param[0] = true;
        param[1] = true;
        param[2] = false;
        Analyze_(doc, param, item);
        boost::unique_lock<boost::mutex> lock(mutex_);
        if(!item.tvalues.empty()||!item.ocvalues.empty())
        {
            for(std::size_t i=0;i<cid_list.size()-1;i++)
            {
                term_t cid = cid_list[i];
                //const Category& c = category_list_[cid];
                //if(c.depth==0&&rv>5) continue;
                //if(c.depth==1&&rv>20) continue;
                //if(c.depth==2&&rv>50) continue;
                term_t child = cid_list[i+1];
                std::ofstream& ofs = *(maxent_ofs_list_[cid]);
                ofs<<child;
                for(std::size_t i=0;i<item.tvalues.size();i++)
                {
                    term_t id = GenNVID_(item.tvalues[i].word, false);
                    ofs<<" "<<id;
                }
                for(std::size_t i=0;i<item.ocvalues.size();i++)
                {
                    term_t id = GenNVID_(item.ocvalues[i].word, true);
                    ofs<<" "<<id;
                }
                ofs<<std::endl;
            }
        }
    }

    void Test_(ScdDocument& doc)
    {
        std::size_t p = p_.fetch_add(1)+1;
        //static boost::mt19937 gen;
        //boost::uniform_int<term_t> dist(1, 100);
        //term_t rv = dist(gen);
        //if(rv!=1) return;
        std::string category;
        std::string title;
        std::string original_category;
        doc.getString("Category", category);
        doc.getString("Title", title);
        doc.getString("OriginalCategory", original_category);
        if(title.empty()) return;
        if(original_category.empty()) return;
        double vprice = ProductPrice::ParseDocPrice(doc, "Price");
        if(vprice<=0.0) return;
        term_t cid=0;
        if(!GetCategoryId_(category, cid)) return;
        //if(boost::algorithm::starts_with(category, book_category_)) return;
        int64_t level = 3;
        int64_t method = 1;
        doc.property("Level") = level;
        doc.property("Method") = method;
        std::string tcategory;
        //std::cerr<<"[T-TITLE]"<<title<<std::endl;
        izenelib::util::ClockTimer clocker;
        Evaluate(doc, tcategory);
        double time = clocker.elapsed();
        {
            boost::unique_lock<boost::mutex> lock(time_mutex_);
            time_+=time;
        }
        std::vector<term_t> cid_list;
        GetCategoryIdList_(category, cid_list);
        for(std::size_t i=1;i<=cid_list.size();i++)
        {
            recall_[i]->second.fetch_add(1);
        }
        //std::cerr<<"[TCATEGORY]"<<tcategory<<std::endl;
        //std::cerr<<"[CATEGORY]"<<category<<std::endl;
        if(!tcategory.empty())
        {
            std::vector<term_t> tcid_list;
            GetCategoryIdList_(tcategory, tcid_list);
            while(!tcid_list.empty())
            {
                if(tcid_list.size()<=cid_list.size())
                {
                    term_t depth = tcid_list.size();
                    accuracy_[depth]->second.fetch_add(1);
                    recall_[depth]->first.fetch_add(1);
                    if(boost::algorithm::starts_with(cid_list, tcid_list))
                    {
                        accuracy_[depth]->first.fetch_add(1);
                    }
                }
                tcid_list.resize(tcid_list.size()-1);
            }
            //if(cid_list!=tcid_list && boost::algorithm::starts_with(tcid_list, cid_list)) return;
            //term_t depth = category_list_[cid].depth;
            //accuracy.second = accuracy_.second.fetch_add(1)+1;
            //if(IsMatch_(category, tcategory))
            //{
            //    accuracy.first = accuracy_.first.fetch_add(1)+1;
            //}
            //else
            //{
            //    accuracy.first = accuracy_.first;
            //    boost::unique_lock<boost::mutex> lock(mutex_);
            //    CategoryCount::iterator it = error_.find(cid);
            //    if(it==error_.end()) error_.insert(std::make_pair(cid, 1));
            //    else it->second+=1;
            //}
        }
        if(p-lastp_>=1000000)
        {
            for(std::size_t i=1;i<=3;i++)
            {
                double v = (double)recall_[i]->first/recall_[i]->second;
                std::cerr<<"[STAT-RECALL]("<<i<<")"<<recall_[i]->first<<","<<recall_[i]->second<<","<<v<<std::endl;
            }
            for(std::size_t i=1;i<=3;i++)
            {
                double vaccuracy = (double)accuracy_[i]->first/accuracy_[i]->second;
                std::cerr<<"[STAT-ACCURACY]("<<i<<")"<<accuracy_[i]->first<<","<<accuracy_[i]->second<<","<<vaccuracy<<std::endl;
            }
            double average_time = time_/recall_[1]->second;
            double qps = (double)recall_[1]->second/time_;
            std::cerr<<"[STAT-TIME]"<<time_<<","<<average_time<<","<<qps<<std::endl;
            lastp_ = p;
        }
    }

    bool JointNgramProcess_(Ngram& ngram)
    {
        uint32_t freq = ngram.dfreq;
        if(freq<30) return false;
        return true;
    }

    bool NgramProcess_(Ngram& ngram)
    {
        //return;
        //term_t type = ngram.type;
        uint32_t freq = ngram.dfreq;
        //if(ngram.freq>ngram.dfreq)
        //{
        //    std::cerr<<"[DNGRAM]"<<ngram.GetText()<<","<<ngram.dfreq<<","<<ngram.freq<<std::endl;
        //}
        //std::cerr<<"[NGRAMTYPE]"<<ngram.type<<std::endl;
        if(freq<30) return false;
        std::string text = ngram.GetText();
        UString ut(text, UString::UTF_8);
        if(ut.length()<2) return false;
        return true;
        //bool is_attrib = false;
        //AttributeMap::const_iterator ait = attribute_map_.lower_bound(ngram.word);
        //if(ait!=attribute_map_.end())
        //{
        //    if(boost::algorithm::starts_with(ait->first, ngram.word))
        //    {
        //        double ratio = (double)ngram.word.size()/ait->first.size();
        //        if(ratio>=0.8) 
        //        {
        //            is_attrib = true;
        //        }
        //    }
        //}

        //if(!is_attrib)
        //{
        //    if(ngram.word.size()<2)
        //    {
        //        return false;
        //    }
        //}
        //else
        //{
        //    return true;
        //}

        //NgramValue nv(ngram);
        //NgramProcessor::CalculateCategoryProb(nv, category_list_);
        //std::pair<term_t, double> max(0, 0.0);
        //for(NgramValue::const_iterator it = nv.begin();it!=nv.end();++it)
        //{
        //    term_t cid = it->first;
        //    double score = it->second.prob;
        //    const Category& c = category_list_[cid];
        //    if(c.parent_cid!=0) continue;
        //    if(score>max.second) 
        //    {
        //        max.second = score;
        //        max.first = cid;
        //    }
        //}
        //bool cf_prop_valid = false;
        //if(freq<=100)
        //{
        //    if(max.second>=0.5) cf_prop_valid = true;
        //}
        //else if(freq<=1000)
        //{
        //    if(max.second>=0.4) cf_prop_valid = true;
        //}
        //else
        //{
        //    if(max.second>=0.3) cf_prop_valid = true;
        //}
        //if(!cf_prop_valid) return false;
        //return true;
    }
    bool NgramFilter_(const NgramValue& nv, const std::string& text)
    {
        UString ut(text, UString::UTF_8);
        if(ut.length()<2) return false;
        NgramValue::const_iterator root_it = nv.find(0);
        const NCI& nci = root_it->second;
        std::size_t freq = nci.freq;
        //std::cerr<<"[FREQ]"<<freq<<std::endl;
        if(freq<50) return false;
        bool is_attrib = false;
        double en_limit = 1.2;
        term_t first_level_limit = 10;
        double max_prop_limit = 1e-7;
        double max_ratio_limit = 0.4;
        AttributeMap::const_iterator ait = attribute_map_.lower_bound(nv.word);
        if(ait!=attribute_map_.end())
        {
            if(boost::algorithm::starts_with(ait->first, nv.word))
            {
                double ratio = (double)nv.word.size()/ait->first.size();
                if(ratio>=0.8) 
                {
                    const AttributeMapValue& av = ait->second;
                    for(std::size_t i=0;i<av.attribute_posting.size();i++)
                    {
                        const std::string& name = av.attribute_posting[i].attribute_name;
                        if(name=="品牌" || name=="型号" || name=="类目关键词")
                        {
                            is_attrib = true;
                            break;
                        }
                    }
                }
            }
        }
        bool all_cn = true;
        for(std::size_t i=0;i<ut.length();i++)
        {
            if(!ut.isChineseChar(i))
            {
                all_cn = false;
                break;
            }
        }

        if(!is_attrib)
        {
            if(nv.word.size()<2)
            {
                return false;
            }
            for(std::size_t i=0;i<ut.length();i++)
            {
                if(ut.isNumericChar(i))
                {
                    return false;
                }
            }
            //if(all_cn && ut.length()<3) return false;
        }
        else
        {
            bool all_num = true;
            for(std::size_t i=0;i<ut.length();i++)
            {
                if(!ut.isNumericChar(i))
                {
                    all_num = false;
                    break;
                }
            }
            if(all_num) return false;
            //else return true;
            en_limit = 1.5;
            first_level_limit = 12;
            max_ratio_limit = 0.3;
        }
        CategoryScore cid_score;
        const Category& pc = category_list_[0];
        const std::vector<term_t>& target_cids = pc.children;
        for(std::size_t j=0;j<target_cids.size();j++)
        {
            term_t cid = target_cids[j];
            //std::cerr<<"[CID]"<<cid<<std::endl;
            //const Category& c = category_list_[cid];
            cid_score[cid] = 0.0;
        }
        double prop_sum = 0.0;
        double max_prop = 0.0;
        for(std::size_t j=0;j<target_cids.size();j++)
        {
            term_t cid = target_cids[j];
            NgramValue::const_iterator it = nv.find(cid);
            if(it==nv.end()) continue;
            const NCI& nci = it->second;
            //double score = nv.category_distrib[n].second;
            std::size_t count = nci.freq;
            const Category& c = category_list_[cid];
            double prop = (double)count/c.offer_count;
            //std::cerr<<"[D]"<<cid<<","<<c.name<<","<<count<<","<<c.offer_count<<std::endl;
            if(prop>max_prop) max_prop = prop;
            prop_sum += prop;
            cid_score[cid] = prop;
        }
        //std::cerr<<"[S0]"<<max_prop<<","<<prop_sum<<std::endl;
        if(max_prop<max_prop_limit) return false;
        double max_ratio = max_prop/prop_sum;
        if(max_ratio<max_ratio_limit) return false;
        double en = 0.0;
        term_t first_level_count = 0;
        for(CategoryScore::iterator it = cid_score.begin();it!=cid_score.end();++it)
        {
            it->second /= prop_sum;
            double p = it->second;
            if(p>0.0)
            {
                en += p*std::log(p)/std::log(2.0);
                ++first_level_count;
            }
        }
        en *= -1.0;
        //std::cerr<<"[S]"<<first_level_count<<","<<en<<","<<max_prop<<","<<prop_sum<<std::endl;
        if(first_level_count>first_level_limit) return false;
        if(en>en_limit) return false;
        //if(ut.length()==2 && en>0.5) return false;
        return true;

    }
    bool DistributionFilter_(const NgramValue& nv, double en_limit_enlarge = 1.0)
    {
        CategoryScore cid_score;
        for(std::size_t i=1;i<category_list_.size();i++)
        {
            const Category& c = category_list_[i];
            term_t cid = c.cid;
            if(c.depth!=2) continue;
            cid_score[cid] = 0.0;
        }
        double csize = cid_score.size();
        double en_max = std::log(csize)/std::log(2.0);
        double en_limit = en_max*0.3*en_limit_enlarge;
        double prop_sum = 0.0;
        double max_prop = 0.0;
        for(std::size_t i=1;i<category_list_.size();i++)
        {
            const Category& c = category_list_[i];
            term_t cid = c.cid;
            if(c.depth!=2) continue;
            NgramValue::const_iterator it = nv.find(cid);
            if(it==nv.end()) continue;
            const NCI& nci = it->second;
            //double score = nv.category_distrib[n].second;
            std::size_t count = nci.freq;
            double prop = (double)count/c.offer_count;
            //std::cerr<<"[D]"<<cid<<","<<c.name<<","<<count<<","<<c.offer_count<<std::endl;
            if(prop>max_prop) max_prop = prop;
            prop_sum += prop;
            cid_score[cid] = prop;
        }
        double max_prop_limit = 1e-5;
        //std::cerr<<"[S0]"<<max_prop<<","<<prop_sum<<std::endl;
        double en = 0.0;
        for(CategoryScore::iterator it = cid_score.begin();it!=cid_score.end();++it)
        {
            it->second /= prop_sum;
            double p = it->second;
            if(p>0.0)
            {
                en += p*std::log(p)/std::log(2.0);
            }
        }
        en *= -1.0;
        //std::cerr<<"[ENLIMIT]"<<text<<","<<cid_score.size()<<","<<en_max<<","<<en_limit<<","<<en<<","<<max_prop<<std::endl;
        if(max_prop<max_prop_limit) return false;
        //std::cerr<<"[S]"<<first_level_count<<","<<en<<","<<max_prop<<","<<prop_sum<<std::endl;
        if(en>en_limit) return false;
        //if(ut.length()==2 && en>0.5) return false;
        return true;
    }
    bool NgramFilter2_(const NgramValue& nv, const std::string& text)
    {
        UString ut(text, UString::UTF_8);
        if(ut.length()<2) return false;
        NgramValue::const_iterator root_it = nv.find(0);
        const NCI& nci = root_it->second;
        std::size_t freq = nci.freq;
        //std::cerr<<"[FREQ]"<<freq<<std::endl;
        if(freq<50) return false;
        bool is_attrib = false;
        AttributeMap::const_iterator ait = attribute_map_.lower_bound(nv.word);
        if(ait!=attribute_map_.end())
        {
            if(boost::algorithm::starts_with(ait->first, nv.word))
            {
                double ratio = (double)nv.word.size()/ait->first.size();
                if(ratio>=0.8) 
                {
                    const AttributeMapValue& av = ait->second;
                    for(std::size_t i=0;i<av.attribute_posting.size();i++)
                    {
                        const std::string& name = av.attribute_posting[i].attribute_name;
                        if(name=="品牌" || name=="型号" || name=="类目关键词")
                        {
                            is_attrib = true;
                            break;
                        }
                    }
                }
            }
        }
        bool all_cn = true;
        for(std::size_t i=0;i<ut.length();i++)
        {
            if(!ut.isChineseChar(i))
            {
                all_cn = false;
                break;
            }
        }

        double en_limit_enlarge = 1.0;
        if(!is_attrib)
        {
            if(nv.word.size()<2)
            {
                return false;
            }
            for(std::size_t i=0;i<ut.length();i++)
            {
                if(ut.isNumericChar(i))
                {
                    return false;
                }
            }
            //if(all_cn && ut.length()<3) return false;
        }
        else
        {
            bool all_num = true;
            for(std::size_t i=0;i<ut.length();i++)
            {
                if(!ut.isNumericChar(i))
                {
                    all_num = false;
                    break;
                }
            }
            if(all_num) return false;
            //else return true;
            en_limit_enlarge = 1.2;
        }
        return DistributionFilter_(nv, en_limit_enlarge);
        CategoryScore cid_score;
        for(std::size_t i=1;i<category_list_.size();i++)
        {
            const Category& c = category_list_[i];
            term_t cid = c.cid;
            if(c.depth!=2) continue;
            cid_score[cid] = 0.0;
        }
        double csize = cid_score.size();
        double en_max = std::log(csize)/std::log(2.0);
        double en_limit = en_max*0.3*en_limit_enlarge;
        double prop_sum = 0.0;
        double max_prop = 0.0;
        for(std::size_t i=1;i<category_list_.size();i++)
        {
            const Category& c = category_list_[i];
            term_t cid = c.cid;
            if(c.depth!=2) continue;
            NgramValue::const_iterator it = nv.find(cid);
            if(it==nv.end()) continue;
            const NCI& nci = it->second;
            //double score = nv.category_distrib[n].second;
            std::size_t count = nci.freq;
            double prop = (double)count/c.offer_count;
            //std::cerr<<"[D]"<<cid<<","<<c.name<<","<<count<<","<<c.offer_count<<std::endl;
            if(prop>max_prop) max_prop = prop;
            prop_sum += prop;
            cid_score[cid] = prop;
        }
        double max_prop_limit = 1e-5;
        //std::cerr<<"[S0]"<<max_prop<<","<<prop_sum<<std::endl;
        double en = 0.0;
        for(CategoryScore::iterator it = cid_score.begin();it!=cid_score.end();++it)
        {
            it->second /= prop_sum;
            double p = it->second;
            if(p>0.0)
            {
                en += p*std::log(p)/std::log(2.0);
            }
        }
        en *= -1.0;
        //std::cerr<<"[ENLIMIT]"<<text<<","<<cid_score.size()<<","<<en_max<<","<<en_limit<<","<<en<<","<<max_prop<<std::endl;
        if(max_prop<max_prop_limit) return false;
        //std::cerr<<"[S]"<<first_level_count<<","<<en<<","<<max_prop<<","<<prop_sum<<std::endl;
        if(en>en_limit) return false;
        //if(ut.length()==2 && en>0.5) return false;
        return true;

    }

    bool GetCategoryId_(const std::string& c, term_t& cid)
    {
        std::string cs = c;
        if(boost::algorithm::ends_with(cs, ">"))
        {
            cs = cs.substr(0, cs.length()-1);
        }
        CategoryIndex::const_iterator it = category_index_.find(cs);
        if(it==category_index_.end()) return false;
        cid = it->second;
        return true;
    }

private:
    idmlib::util::IDMAnalyzer* analyzer_;
    std::vector<ScdDocument> spu_list_;
    boost::atomic<term_t> oid_;
    AttributeMap attribute_map_;
    NgramProcessor* ngram_processor_;
    Writer* dic_writer_;
    NgramProcessor::NgramDic* dic_;
    JointDic* joint_dic_;
    std::vector<std::ofstream*> maxent_ofs_list_;
    std::vector<MaxentClassifier*> me_classifiers_;
    izenelib::util::HashIDTraits<word_t, term_t> hasher_;
    std::vector<Category> category_list_;
    CategoryIndex category_index_;
    std::string book_category_;
    term_t book_cid_;
    std::size_t dic_size_;
    boost::atomic<std::size_t> p_;
    std::size_t lastp_;
    std::vector<AtomicPair*> recall_;
    std::vector<AtomicPair*> accuracy_;
    //std::pair<boost::atomic<std::size_t>, boost::atomic<std::size_t> > accuracy1_;
    //std::pair<boost::atomic<std::size_t>, boost::atomic<std::size_t> > accuracy2_;
    //std::pair<boost::atomic<std::size_t>, boost::atomic<std::size_t> > accuracy3_;
    double time_;
    CategoryCount error_;
    boost::mutex mutex_;
    boost::mutex time_mutex_;

};

NS_IDMLIB_B5M_END

#endif

