#ifndef IDMLIB_B5M_OMAPPERGENERATOR_H_
#define IDMLIB_B5M_OMAPPERGENERATOR_H_
#include "b5m_helper.h"
#include "b5m_types.h"
#include "scd_doc_processor.h"
#include "product_price.h"
#include "original_mapper.h"

NS_IDMLIB_B5M_BEGIN

class OmapperGenerator{
public:
    typedef std::pair<std::string, std::string> Key;
    typedef boost::unordered_map<std::string, std::size_t> Value;
    typedef boost::unordered_map<Key, Value> Data;
    OmapperGenerator()
    {
    }

    bool Generate(const std::string& scd, const std::string& edata, const std::string& output, int thread_num=1)
    {
        if(!om_.OpenFile(edata)) return false;
        classifier_.reset(new msgpack::rpc::client("10.10.96.2", 18299));
        count_ = 0;
        p_ = 0;
        LOG(INFO)<<"output path "<<output<<std::endl;
        boost::filesystem::create_directories(output);
        ScdDocProcessor::ProcessorType p = boost::bind(&OmapperGenerator::Process_, this, _1);
        ScdDocProcessor sd_processor(p, thread_num);
        //sd_processor.SetLimit(250ul);
        sd_processor.AddInput(scd);
        sd_processor.Process();
        LOG(INFO)<<"process finished"<<std::endl;
        LOG(INFO)<<"p value "<<p_<<std::endl;
        typedef std::pair<std::size_t, std::string> KValue;
        std::vector<KValue> kv_list;
        std::string data_file = output+"/data";
        std::string result_file = output+"/result";
        std::ofstream ofsd(data_file.c_str());
        std::ofstream ofsr(result_file.c_str());
        for(Data::const_iterator it = data_.begin();it!=data_.end();++it)
        {
            const Key& key = it->first;
            const Value& value = it->second;
            kv_list.resize(0);
            for(Value::const_iterator vit = value.begin();vit!=value.end();++vit)
            {
                KValue kvalue(vit->second, vit->first);
                kv_list.push_back(kvalue);
            }
            std::sort(kv_list.begin(), kv_list.end(), std::greater<KValue>());
            for(std::size_t i=0;i<kv_list.size();i++)
            {
                ofsd<<key.first<<","<<key.second<<":"<<kv_list[i].second<<","<<kv_list[i].first<<std::endl;
                if(i==0)
                {
                    ofsr<<key.first<<","<<key.second<<":"<<kv_list[i].second<<std::endl;
                }
            }
        }
        ofsd.close();
        ofsr.close();
        return true;
    }

private:
    void Process_(ScdDocument& doc)
    {
        std::string oc;
        std::string source;
        doc.getString("OriginalCategory", oc);
        doc.getString("Source", source);
        if(oc.empty()||source.empty()) return;
        std::string category;
        om_.Map(source, oc, category);
        if(!category.empty()) return;
        std::size_t p = p_.fetch_add(1);
        if(p%100000==0)
        {
            LOG(INFO)<<"p "<<p<<std::endl;
        }
        std::string title;
        doc.getString("Title", title);
        if(title.empty()) return;
        double price = ProductPrice::ParseDocPrice(doc);
        if(price<0.0) price = 0.0;
        //std::cerr<<"["<<p<<"]title "<<title<<","<<boost::this_thread::get_id()<<std::endl;
        category = classifier_->call("classify",title, oc, price, 3, 2).get<std::string>();
        //std::cerr<<"["<<p<<"]category "<<category<<boost::this_thread::get_id()<<std::endl;
        if(category.empty()) return;
        Key key(source, oc);
        boost::unique_lock<boost::mutex> lock(mutex_);
        Data::iterator it = data_.find(key);
        if(it==data_.end())
        {
            Value v;
            v.insert(std::make_pair(category, 1));
            data_.insert(std::make_pair(key, v));
        }
        else
        {
            Value& v = it->second;
            Value::iterator vit = v.find(category);
            if(vit==v.end())
            {
                v.insert(std::make_pair(category, 1));
            }
            else
            {
                vit->second+=1;
            }
        }
        count_++;
        if(count_%100000==0)
        {
            LOG(INFO)<<"Omapper predict "<<count_<<std::endl;
        }
    }

private:
    OriginalMapper om_;
    Data data_;
    boost::mutex mutex_;
    boost::shared_ptr<msgpack::rpc::client> classifier_;
    std::size_t count_;
    boost::atomic<std::size_t> p_;

};

NS_IDMLIB_B5M_END

#endif

