#ifndef IDMLIB_B5M_SPUMATCHER_H_
#define IDMLIB_B5M_SPUMATCHER_H_
#include "b5m_helper.h"
#include "b5m_types.h"
#include "scd_doc_processor.h"
#include "b5m_helper.h"
#include "product_price.h"
#include <sf1common/ScdParser.h>
#include <sf1common/ScdWriter.h>
#include <am/sequence_file/ssfr.h>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <3rdparty/msgpack/rpc/client.h>
#include <cmath>

//#define SM_DEBUG

NS_IDMLIB_B5M_BEGIN

using izenelib::util::UString;

class SpuMatcher {
    typedef std::vector<uint32_t> word_t;
    typedef std::pair<uint32_t, UString> Term;
    typedef std::vector<Term> Word;
    struct ForwardOccur {
        ForwardOccur(){}
        ForwardOccur(uint32_t p1, const std::string& p2, uint32_t p3, uint32_t p4)
        :pid(p1), key(p2), aindex(p3), vindex(p4)
        {
        }
        uint32_t pid;
        std::string key;
        uint32_t aindex;//attribute index
        uint32_t vindex;//value index of attribute in aindex
    };
    struct ForwardItem {
        ForwardItem(){}
        ForwardItem(const Word& p1, const std::string& p2)
        : word(p1), text(p2)
        {
        }
        Word word;
        std::string text;
        std::vector<ForwardOccur> occurs;
    };
    typedef boost::unordered_map<word_t, uint32_t> ForwardIndex;
    typedef std::vector<ForwardItem> Forward;
    struct InvertItem {
        InvertItem(){}
        InvertItem(uint32_t p1, uint32_t p2)
        :kid(p1), pos(p2)
        {
        }
        uint32_t kid;//keyword id
        uint32_t pos;//position in keyword
    };
    typedef std::pair<uint32_t, uint32_t> InvertKey;//termid, cid
    typedef std::vector<InvertItem> InvertValue;
    typedef std::map<InvertKey, InvertValue> Invert;
    //for one SPU
    //struct MatchItem {
    //    std::string key;
    //    std::vector<uint32_t> occurs;
    //};
    typedef std::pair<uint32_t, std::string> MatchKey;//fid and key
    typedef std::vector<uint32_t> Occurs;
    typedef std::vector<std::vector<uint32_t> > MultiOccurs;
    struct MatchValue {
        MatchValue()
        : aindex(0), vindex(0), score(0.0)
        {
        }
        MultiOccurs occurs;
        uint32_t aindex;
        uint32_t vindex;
        double score;
    };
    struct FMatchValue {
        FMatchValue():aindex(0), vindex(0)
        {
        }
        MultiOccurs occurs;
        uint32_t aindex;
        uint32_t vindex;
    };
    typedef boost::unordered_map<MatchKey, MatchValue> Match; 
    struct MatchResult {
        uint32_t pid;
        std::string docid;
        double score;
        bool operator<(const MatchResult& another) const
        {
            return score>another.score;
        }
    };
    struct MatchItem {
        MatchItem():pid(0), fid(0), aindex(0), vindex(0), occur(0,0)
        {
        }
        MatchItem(uint32_t p1, const std::string& p2, uint32_t p3, uint32_t p4, uint32_t p5, const std::pair<uint32_t, uint32_t>& p6):pid(p1), key(p2), fid(p3), aindex(p4), vindex(p5), occur(p6)
        {
        }
        uint32_t pid;
        std::string key;
        uint32_t fid;
        uint32_t aindex;
        uint32_t vindex;
        std::pair<uint32_t,uint32_t> occur;
        bool operator<(const MatchItem& y) const
        {
            if(pid!=y.pid) return pid<y.pid;
            if(fid!=y.fid) return fid<y.fid;
            return occur.first<y.occur.first;
        }
    };
    struct AttribValue {
        AttribValue(){}
        AttribValue(const std::string& p1, const std::vector<std::string>& p2, uint32_t p3)
        : display(p1), match(p2), aindex(p3)
        {
        }
        std::string display;
        std::vector<std::string> match;
        uint32_t aindex;
    };
    typedef std::vector<AttribValue> AttribValueList;
    typedef boost::unordered_map<std::string, AttribValueList> AttribMap;
    typedef std::set<word_t> AccessoryDic;
public:
    struct SpuProduct {
        SpuProduct():pid(0), cid(0){}
        SpuProduct(const std::string& p1, const std::string& p2, const std::string& p3, const ProductPrice& p4)
        :pid(0),docid(p1), title(p2), category(p3), cid(0), price(p4)
        {
        }
        uint32_t pid;
        std::string docid;
        std::string title;
        std::string category;
        uint32_t cid;
        ProductPrice price;
        AttribMap am;
        std::vector<Attribute> dattributes;
        void add_attrib(const std::string& name, const std::string& display_value, const std::vector<std::string>& match_values)
        {
            uint32_t aindex = 0;
            if(name=="brand" || name=="model")
            {
                aindex = 0;
            }
            else if(am.find(name)!=am.end())
            {
                aindex = am.size()-2;
            }
            else aindex = am.size()-1;
            AttribValue av(display_value, match_values, aindex);
            am[name].push_back(av);
        }
        void get_brand(std::string& value) const
        {
            AttribMap::const_iterator it = am.find("brand");
            if(it==am.end()) return;
            if(it->second.empty()) return;
            value = it->second[0].display;
        }
    };
    SpuMatcher()
    {
        idmlib::util::IDMAnalyzerConfig csconfig = idmlib::util::IDMAnalyzerConfig::GetCommonConfig("","", "");
        csconfig.symbol = true;
        analyzer_ = new idmlib::util::IDMAnalyzer(csconfig);
#ifdef SM_DEBUG
        debug_ = 0;
#endif
    }

    ~SpuMatcher()
    {
        delete analyzer_;
    }

    bool Load(const std::string& knowledge)
    {
        std::string cfile = knowledge+"/category";
        B5MHelper::LoadCategories(cfile, cm_);
        std::string abfile = knowledge+"/accessory_brand";
        LoadAccessoryBrand_(abfile);
        LOG(INFO)<<"Accessory brand count "<<adic_.size()<<std::endl;
        std::string spu_json_file = knowledge+"/spu.json";
        Json::Reader json_reader;
        std::ifstream ifs(spu_json_file.c_str());
        std::string line;
        using Json::Value;
        products_.resize(1);
        while( getline(ifs, line))
        {
            Value value;
            json_reader.parse(line, value);
            std::string docid = value["DOCID"].asString();
            std::string category = value["category"].asString();
            uint32_t cid=0;
            if(!cm_.GetCategoryId_(category, cid)) continue;
            std::string sprice;
            if(value["price"].isString())
            {
                sprice = value["price"].asString();
            }
            else
            {
                double dprice = value["price"].asDouble();
                sprice = boost::lexical_cast<std::string>(dprice);
            }
            ProductPrice price;
            price.Parse(sprice);
            SpuProduct product(docid, "", category, price);
            product.cid = cid;
            const Value& vattributes = value["attributes"];
            std::string dbrand;
            std::string dmodel;
            for(std::size_t i=0;i<vattributes.size();i++)
            {
                const Value& vattribute = vattributes[i];
                const std::string& name = vattribute["name"].asString();
                const Value& vvalue = vattribute["value"];
                if(!vvalue.isArray())
                {
                    const std::string& display = vvalue.asString();
                    if(name=="brand") dbrand = display;
                    else if(name=="model") dmodel = display;
                    std::vector<std::string> match(1, display);
                    product.add_attrib(name, display, match);
                }
                else
                {
                    const Value& vv = vvalue[(std::size_t)0];
                    const std::string& display = vv.asString();
                    if(name=="brand") dbrand = display;
                    else if(name=="model") dmodel = display;
                    std::vector<std::string> match;
                    if(vvalue.size()==1)
                    {
                        match.push_back(display);
                    }
                    else
                    {
                        for(std::size_t j=1;j<vvalue.size();j++)
                        {
                            match.push_back(vvalue[j].asString());
                        }
                    }
                    product.add_attrib(name, display, match);
                }
            }
            const Value& vdattributes = value["dattributes"];
            if(!vdattributes.isNull())
            {
                for(std::size_t i=0;i<vdattributes.size();i++)
                {
                    const Value& vdattribute = vdattributes[i];
                    const std::string& name = vdattribute["name"].asString();
                    const std::string& value = vdattribute["value"].asString();
                    Attribute a;
                    a.name = name;
                    a.values.push_back(value);
                    a.is_optional = false;
                    product.dattributes.push_back(a);
                }
            }
            std::string title = dbrand+" "+dmodel;
            product.title = title;
            uint32_t pid = products_.size();
            product.pid = pid;
            products_.push_back(product);
            //std::vector<std::string> brands;//include synonym
            //std::vector<std::string> models;
            //std::vector<std::pair<std::string, std::string> > occur_value;
            //const Value& vbrand = value["brand"];
            //if(vbrand.isArray())
            //{
            //    for(std::size_t i=0;i<vbrand.size();i++)
            //    {
            //        brands.push_back(vbrand[i].asString());
            //    }
            //}
            //else {
            //    brands.push_back(vbrand.asString());
            //}
            //const Value& vmodel = value["model"];
            //if(vmodel.isArray())
            //{
            //    for(std::size_t i=0;i<vmodel.size();i++)
            //    {
            //        models.push_back(vmodel[i].asString());
            //    }
            //}
            //else {
            //    models.push_back(vmodel.asString());
            //}
            //for(std::size_t i=0;i<brands.size();i++)
            //{
            //    occur_value.push_back(std::make_pair("brand", brands[i]));
            //}
            //for(std::size_t i=0;i<models.size();i++)
            //{
            //    occur_value.push_back(std::make_pair("model", models[i]));
            //}
            //Value& vsubs = value["subs"];
            //std::vector<std::string> sub_keys(vsubs.size());
            //std::vector<std::vector<std::string> sub_values(vsubs.size());
            //for(std::size_t i=0;i<vsub.size();i++)
            //{
            //    Value& vsub = vsubs[i];
            //    sub_keys[i] = vsub["key"].asString();
            //    Value& vsubvalue = vsub["value"];
            //    for(std::size_t j=0;j<vsubvalue.size();j++)
            //    {
            //        sub_values[i].push_back(vsubvalue[j].asString());
            //        occur_value.push_back(std::make_pair(sub_keys[i], sub_values[i][j]));
            //    }
            //}
            for(AttribMap::const_iterator ait = product.am.begin();ait!=product.am.end();++ait)
            {
                const std::string& key = ait->first;
                const AttribValueList& avl = ait->second;
                for(std::size_t a=0;a<avl.size();a++)
                {
                    const AttribValue& av = avl[a];
                    uint32_t vindex = a+1;
                    uint32_t aindex = av.aindex;
                    const std::vector<std::string>& values = av.match;
                    for(std::size_t i=0;i<values.size();i++)
                    {
                        const std::string& value = values[i];
                        word_t iword;
                        Word word;
                        GetWordS_(value, word, iword);
                        uint32_t fid = 0;
                        ForwardIndex::iterator fit = forward_index_.find(iword);
                        if(fit==forward_index_.end())
                        {
                            fid = forward_.size();
                            forward_index_.insert(std::make_pair(iword, fid));
                            ForwardItem fi(word, value);
                            forward_.push_back(fi);
                        }
                        else fid = fit->second;
                        ForwardItem& forward = forward_[fid];
                        ForwardOccur fo(pid, key, aindex, vindex);
                        forward.occurs.push_back(fo);
                        for(std::size_t w=0;w<word.size();w++)
                        {
                            uint32_t term = word[w].first;
                            InvertKey key(term, cid);
                            invert_[key].push_back(InvertItem(fid, w));
                            uint32_t tcid = cid;
                            uint32_t pcid=0;
                            while(GetParentCid_(tcid, pcid))
                            {
                                InvertKey key(term, pcid);
                                invert_[key].push_back(InvertItem(fid, w));
                                tcid = pcid;
                            }
                        }
                    }
                }
            }
        }
        ifs.close();
        LOG(INFO)<<products_.size()-1<<" SPUs loaded"<<std::endl;
        return true;
    }
    void SetBrandExtractor(const boost::shared_ptr<msgpack::rpc::client>& be)
    {
        be_ = be;
    }
    bool Evaluate(const Document& doc, SpuProduct& product)
    {
#ifdef SM_DEBUG
        //debug_++;
        //if(debug_%100==0)
        //{
        //    LOG(INFO)<<"[D]"<<debug_<<std::endl;
        //}
#endif
        std::string docid;
        doc.getString("DOCID", docid);
        std::string category;
        doc.getString("Category", category);
        uint32_t cid=0;
        if(!cm_.GetCategoryId_(category, cid)) return false;
        std::string title;
        doc.getString("Title", title);
        double price = ProductPrice::ParseDocPrice(doc);
#ifdef SM_DEBUG
        std::cerr<<"[T]"<<title<<","<<category<<std::endl;
#endif
        boost::unordered_set<std::string> brand_strict;
        std::string applyto;
        if(B5MHelper::Is3cAccessoryCategory(category))
        {
            std::vector<std::string> brands;
            B5MHelper::GetBrandAndApplyto(doc, brands, applyto);
            for(std::size_t i=0;i<brands.size();i++)
            {
                boost::to_lower(brands[i]);
                brand_strict.insert(brands[i]);
            }
            boost::to_lower(applyto);
            if(brand_strict.empty())
            {
                Word word;
                word_t iword;
                GetWordS_(title, word, iword);
                word_t w;
                w.reserve(word.size());
                bool found = false;
                for(std::size_t i=0;i<word.size();i++)
                {
                    w.push_back(word[i].first);
                    AccessoryDic::const_iterator it = adic_.lower_bound(w);
                    if(it==adic_.end()) 
                    {
                        w.resize(w.size()-1);
                        break;
                    }
                    else if(boost::algorithm::starts_with(*it, w))
                    {
                        if(*it==w)
                        {
                            found = true;
                        }
                    }
                    else
                    {
                        w.resize(w.size()-1);
                        break;
                    }
                }
                if(found)
                {
                    std::string fb;
                    for(std::size_t i=0;i<w.size();i++)
                    {
                        std::string str;
                        word[i].second.convertString(str, UString::UTF_8);
                        fb+=str;
                    }
                    boost::to_lower(fb);
                    brand_strict.insert(fb);
                    //LOG(INFO)<<"found fb "<<fb<<","<<docid<<std::endl;
                }
            }
        }
        //if(B5MHelper::Is3cAccessoryCategory(category)&&be_)
        //{
        //    msgpack::rpc::future f = be_->call("brand", title, category);
        //    std::string bs = f.get<std::string>();
        //    std::vector<std::string> vec;
        //    boost::algorithm::split(vec, bs, boost::is_any_of("/"));
        //    for(std::size_t i=0;i<vec.size();i++)
        //    {
        //        brand_strict.insert(vec[i]);
        //    }
        //}
#ifdef SM_DEBUG
        //std::cerr<<"cid "<<cid<<std::endl;
#endif
        Word word;
        word_t iword;
        GetWordS_(title, word, iword);
        typedef boost::unordered_map<uint32_t, FMatchValue> FMatchMap;//key is fid
        FMatchMap fmm;
        for(std::size_t i=0;i<word.size();i++)
        {
            uint32_t term = word[i].first;
            uint32_t tpos = i+1;
            InvertKey key(term, cid);
            Invert::const_iterator iit = invert_.find(key);
            if(iit==invert_.end()) continue;
            const InvertValue& iv = iit->second;
            for(std::size_t j=0;j<iv.size();j++)
            {
                const InvertItem& pi = iv[j];
                uint32_t kpos = pi.pos;
                FMatchValue& fmv = fmm[pi.kid];
                const ForwardItem& fi = forward_[pi.kid];
                if(fmv.occurs.empty())
                {
                    //fmv.occurs.resize(fi.word.size(), 0);
                    fmv.occurs.resize(fi.word.size());
                }
                //fmv.occurs[kpos] = tpos;
                fmv.occurs[kpos].push_back(tpos);
                //for(std::size_t l=0;l<fi.occurs.size();l++)
                //{
                //    uint32_t pid = fi.occurs[l].pid;
                //    //const SpuProduct& product = products_[pid];
                //    const std::string& key = fi.occurs[l].key;
                //    uint32_t aindex = fi.occurs[l].aindex;
                //    uint32_t vindex = fi.occurs[l].vindex;
                //    MatchItem mi(pid, key, pi.kid, aindex, vindex, std::make_pair(kpos, tpos));
                //    mi_list.push_back(mi);
                //    //MatchMap::iterator mit = mm.find(pid);
                //    //if(mit==mm.end())
                //    //{
                //    //    mit = mm.insert(std::make_pair(pid, Match())).first;
                //    //}
                //    //Match& m = mit->second;
                //    //MatchKey mk(pi.kid, key);
                //    //MatchValue& mv = m[mk];
                //    //if(mv.occurs.empty()) //not init
                //    //{
                //    //    mv.occurs.resize(fi.word.size(), 0);
                //    //    mv.aindex = aindex;
                //    //    mv.vindex = vindex;
                //    //}
                //    //mv.occurs[kpos] = tpos;
                //}
            }
        }
        typedef boost::unordered_map<uint32_t, Match> MatchMap;//key is spuid
        MatchMap mm;
        uint32_t mcid = cid;
        //{
        //    if(cm_.data[mcid].depth==3)
        //    {
        //        mcid = cm_.data[mcid].parent_cid;
        //    }
        //}
        for(FMatchMap::iterator it = fmm.begin();it!=fmm.end();++it)
        {
            uint32_t fid = it->first;
            FMatchValue& fmv = it->second;
            const ForwardItem& fi = forward_[fid];
            for(std::size_t i=0;i<fi.occurs.size();i++)
            {
                const ForwardOccur& fo = fi.occurs[i];
                uint32_t pid = fo.pid;
                const SpuProduct& product = products_[pid];
                if(!IsParent_(product.cid, mcid)&&!IsParent_(mcid, product.cid)) continue;
                if(price<product.price.Min() || price>product.price.Max()) continue;
                if(!brand_strict.empty()||!applyto.empty())
                {
                    std::string brand;
                    product.get_brand(brand);
                    std::vector<std::string> vec;
                    boost::algorithm::split(vec, brand, boost::is_any_of("/"));
                    for(std::size_t i=0;i<vec.size();i++)
                    {
                        boost::to_lower(vec[i]);
                    }
                    if(!brand_strict.empty())
                    {
                        bool brand_match = false;
                        for(std::size_t i=0;i<vec.size();i++)
                        {
                            if(brand_strict.find(vec[i])!=brand_strict.end())
                            {
                                brand_match = true;
                                break;
                            }
                        }
                        if(!brand_match) continue;
                    }
                    if(!applyto.empty())
                    {
                        bool applyto_match = false;
                        for(std::size_t i=0;i<vec.size();i++)
                        {
                            if(boost::algorithm::starts_with(applyto, vec[i]))
                            {
                                applyto_match = true;
                                break;
                            }
                        }
                        if(applyto_match) continue;
                    }
                }

                const std::string& key = fo.key;
                std::pair<bool, double> mbs = GetMatchScore2_(fi, key, product, fmv.occurs);
                if(!mbs.first) continue;
                double ms = mbs.second;
#ifdef SM_DEBUG
                //std::cerr<<fi.text<<","<<key<<","<<ms<<std::endl;
#endif
#ifdef SM_DEBUG
                //std::cerr<<fi.text<<","<<key<<","<<ms<<std::endl;
                //if(pid==101891)
                //{
                //    std::cerr<<product.title<<","<<fi.text<<","<<key<<","<<ms<<std::endl;
                //}
#endif
                uint32_t aindex = fo.aindex;
                uint32_t vindex = fo.vindex;
                MatchMap::iterator mit = mm.find(pid);
                if(mit==mm.end())
                {
                    mit = mm.insert(std::make_pair(pid, Match())).first;
                }
                Match& m = mit->second;
                MatchKey mk(fid, key);
                MatchValue& mv = m[mk];
                if(mv.occurs.empty()) //not init
                {
                    mv.occurs = fmv.occurs;
                    mv.aindex = aindex;
                    mv.vindex = vindex;
                    std::size_t ioccur=0;
                    for(std::size_t j=0;j<mv.occurs.size();j++)
                    {
                        //if(mv.occurs[j]!=0) ioccur++;
                        if(!mv.occurs[j].empty()) ioccur++;
                    }
                    //mv.score = ms*ioccur;
                    mv.score = ms;
                }
            }
        }
        typedef std::vector<MatchValue> MatchValueList;
        typedef boost::unordered_map<std::string, MatchValueList> KeyScore;
        typedef boost::unordered_map<uint32_t, double> SpuScore;
        SpuScore ss;
        boost::unordered_set<uint32_t> dd_pos;
        for(MatchMap::iterator it = mm.begin();it!=mm.end();++it)
        {
            uint32_t pid = it->first;
            Match& match = it->second;
            KeyScore ks;
            double score = 0.0;
            for(Match::iterator mit = match.begin();mit!=match.end();++mit)
            {
                //uint32_t kid = mit->first.first;
                //const ForwardItem& fi = forward_[kid];
                const std::string& key = mit->first.second;
                MatchValue& mv = mit->second;
                if(key=="brand" || key=="model" || key=="distrib")
                {
                    for(std::size_t i=0;i<mv.occurs.size();i++)
                    {
                        if(!mv.occurs[i].empty())
                        {
                            dd_pos.insert(mv.occurs[i].front());
                        }
                    }
                }
                ks[key].push_back(mv);
                score += mv.score;
            }
            if(ks.find("brand")==ks.end()) continue;
            std::size_t distrib_count=0;
            KeyScore::const_iterator kit = ks.find("distrib");
            if(kit!=ks.end())
            {
                distrib_count = kit->second.size();
            }
            if(ks.find("model")==ks.end()&&distrib_count!=1) continue;
            ss[pid] = score;
        }
        std::pair<double, uint32_t> spu_max(0.0, 0);
        for(SpuScore::const_iterator sit = ss.begin();sit!=ss.end();sit++)
        {
            if(sit->second>spu_max.first)
            {
                spu_max.first = sit->second;
                spu_max.second = sit->first;
            }
        }
        if(spu_max.first<=0.0) return false;
        //std::cerr<<"max spu id "<<spu_max.second<<std::endl;
        MatchMap::iterator it = mm.find(spu_max.second);
        Match& match = it->second;
        KeyScore ks;
        for(Match::iterator mit = match.begin();mit!=match.end();++mit)
        {
            const std::string& key = mit->first.second;
            MatchValue& mv = mit->second;
#ifdef SM_DEBUG
            uint32_t pid = spu_max.second;
            if(pid==101891)
            {
                const SpuProduct& product = products_[pid];
                std::cerr<<product.title<<","<<pid<<","<<key<<std::endl;
            }
#endif
            ks[key].push_back(mv);
        }
        product = products_[spu_max.second];
        std::vector<std::string> extra_titles;
        for(KeyScore::const_iterator kit = ks.begin();kit!=ks.end();++kit)
        {
            const std::string& key = kit->first;
            if(key=="brand" || key=="model") continue;
            const MatchValueList& mvl = kit->second;
            boost::unordered_set<uint32_t> vindex_set;
            for(std::size_t i=0;i<mvl.size();i++)
            {
                const MatchValue& mv = mvl[i];
                vindex_set.insert(mv.vindex);
            }
            if(vindex_set.size()>1) continue;
            const MatchValue& mv = mvl.front();
            if(mv.occurs.size()==1 && dd_pos.find(mv.occurs[0][0])!=dd_pos.end()) continue;
            uint32_t apos = 32u-mv.aindex;
            uint32_t intv = mv.vindex%16;
            char cv;
            if(intv<10)
            {
                cv = '0'+intv;
            }
            else
            {
                cv = 'a'+intv-10;
            }
            product.docid[apos] = cv;
            for(AttribMap::const_iterator ait=product.am.begin();ait!=product.am.end();ait++)
            {
                const AttribValueList& avl = ait->second;
                //std::cerr<<ait->first<<","<<avl.size()<<std::endl;
                if(avl.front().aindex!=mv.aindex) continue;
                if(avl.size()<mv.vindex) break;
                const AttribValue& v = avl[mv.vindex-1];
                extra_titles.push_back(v.display);
                break;
            }
        }
        for(std::size_t i=0;i<extra_titles.size();i++)
        {
            product.title += " "+extra_titles[i];
        }
#ifdef SM_DEBUG
        if(!product.docid.empty())
        {
            std::cerr<<"[R]"<<title<<","<<product.title<<std::endl;
        }
#endif
        return !product.docid.empty();
    }
//    bool Evaluate(const Document& doc, SpuProduct& product)
//    {
//#ifdef SM_DEBUG
//        debug_++;
//        if(debug_%100==0)
//        {
//            LOG(INFO)<<"[D]"<<debug_<<std::endl;
//        }
//#endif
//        std::string category;
//        doc.getString("Category", category);
//        uint32_t cid=0;
//        if(!cm_.GetCategoryId_(category, cid)) return false;
//        std::string title;
//        doc.getString("Title", title);
//        word_t word;
//        GetWordS_(title, word);
//        typedef boost::unordered_map<uint32_t, Match> MatchMap;//key is spuid
//        MatchMap mm;
//        for(std::size_t i=0;i<word.size();i++)
//        {
//            uint32_t term = word[i];
//            uint32_t tpos = i+1;
//            InvertKey key(term, cid);
//            Invert::const_iterator iit = invert_.find(key);
//            if(iit==invert_.end()) continue;
//            const InvertValue& iv = iit->second;
//            for(std::size_t j=0;j<iv.size();j++)
//            {
//                const InvertItem& pi = iv[j];
//                uint32_t kpos = pi.pos;
//                const ForwardItem& fi = forward_[pi.kid];
//                for(std::size_t l=0;l<fi.occurs.size();l++)
//                {
//                    uint32_t pid = fi.occurs[l].pid;
//                    //const SpuProduct& product = products_[pid];
//                    const std::string& key = fi.occurs[l].key;
//                    uint32_t aindex = fi.occurs[l].aindex;
//                    uint32_t vindex = fi.occurs[l].vindex;
//                    MatchMap::iterator mit = mm.find(pid);
//                    if(mit==mm.end())
//                    {
//                        mit = mm.insert(std::make_pair(pid, Match())).first;
//                    }
//                    Match& m = mit->second;
//                    continue;
//                    MatchKey mk(pi.kid, key);
//                    MatchValue& mv = m[mk];
//                    if(mv.occurs.empty()) //not init
//                    {
//                        mv.occurs.resize(fi.word.size(), 0);
//                        mv.aindex = aindex;
//                        mv.vindex = vindex;
//                    }
//                    mv.occurs[kpos] = tpos;
//                }
//            }
//        }
//        for(MatchMap::iterator it = mm.begin();it!=mm.end();++it)
//        {
//            uint32_t pid = it->first;
//            Match& match = it->second;
//            typedef std::vector<MatchValue> MatchValueList;
//            typedef boost::unordered_map<std::string, MatchValueList> KeyScore;
//            KeyScore ks;
//            for(Match::iterator mit = match.begin();mit!=match.end();++mit)
//            {
//                uint32_t kid = mit->first.first;
//                const ForwardItem& fi = forward_[kid];
//                const std::string& key = mit->first.second;
//                MatchValue& mv = mit->second;
//                double ms = GetMatchScore_(fi, key, mv.occurs);
//                if(ms>=0.8)
//                {
//                    mv.score = ms;
//                    ks[key].push_back(mv);
//                }
//            }
//            if(ks.find("brand")==ks.end()) continue;
//            if(ks.find("model")==ks.end()) continue;
//            product = products_[pid];
//            for(KeyScore::const_iterator kit = ks.begin();kit!=ks.end();++kit)
//            {
//                const MatchValueList& mvl = kit->second;
//                if(mvl.size()>1) continue;
//                const MatchValue& mv = mvl.front();
//                uint32_t apos = 32u-mv.aindex;
//                uint32_t intv = mv.vindex%16;
//                char cv;
//                if(intv<10)
//                {
//                    cv = '0'+intv;
//                }
//                else
//                {
//                    cv = 'a'+intv-10;
//                }
//                product.docid[apos] = cv;
//            }
//        }
//        return !product.docid.empty();
//    }
    void Test(const std::string& scd, int thread_num=1)
    {
        ScdDocProcessor::ProcessorType p = boost::bind(&SpuMatcher::Test_, this, _1);
        ScdDocProcessor sd_processor(p, thread_num);
        sd_processor.AddInput(scd);
        sd_processor.Process();
    }
private:
    void Test_(ScdDocument& doc)
    {
        SpuProduct product;
        Evaluate(doc, product);
        std::string title;
        doc.getString("Title", title);
#ifdef SM_DEBUG
        std::cerr<<"[R]"<<title<<","<<product.title<<","<<product.docid<<"["<<product.pid<<"]"<<std::endl;
#endif
    }
    bool GetParentCid_(uint32_t cid, uint32_t& pcid) const
    {
        if(cid==0) return false;
        if(cid>=cm_.data.size()) return false;
        pcid = cm_.data[cid].parent_cid;
        return true;
    }
    bool IsSymbol_(const UString& text) const
    {
        return (!text.isChineseChar(0)&&!text.isAlphaChar(0)&&!text.isDigitChar(0));
    }
    std::pair<uint32_t, uint32_t> InnerScore_(const Occurs& occurs) const
    {
        std::pair<uint32_t, uint32_t> tpos(occurs[0],occurs[0]);
        uint32_t klen = 0;
        for(std::size_t i=0;i<occurs.size();i++)
        {
            if(occurs[i]>0) klen++;
            if(occurs[i]<tpos.first) tpos.first = occurs[i];
            if(occurs[i]>tpos.second) tpos.second = occurs[i];
        }
        uint32_t dist = tpos.second-tpos.first;
        uint32_t iswap = 0;
        Occurs kpos(occurs);
        for(uint32_t i=0;i<kpos.size();i++)
        {
            if(kpos[i]==0) continue;
            for(uint32_t j=i;j<kpos.size()-1;j++)
            {
                if(kpos[j]==0) continue;
                if(kpos[j]>kpos[j+1])
                {
                    std::swap(kpos[j], kpos[j+1]);
                    iswap++;
                }
            }
        }
        return std::make_pair(iswap, dist);
    }
    void InnerMax_(const MultiOccurs& occurs, const std::vector<std::size_t>& indexes, std::vector<std::size_t>& max_indexes, std::pair<uint32_t, uint32_t>& max) const
    {
        std::vector<std::size_t> next_indexes(indexes.size(), 0);
        bool next_set = false;
        Occurs toccurs(occurs.size(), 0);
        for(std::size_t i=0;i<occurs.size();i++)
        {
            const std::vector<uint32_t>& tposes = occurs[i];
            if(tposes.empty()) continue;
            std::size_t index = indexes[i];
            toccurs[i] = tposes[index];
            if(!next_set)
            {
                if(index<tposes.size()-1)
                {
                    next_indexes[i] = index+1;
                    next_set = true;
                }
                else
                {
                    next_indexes[i] = index;
                }
            }
            else
            {
                next_indexes[i] = index;
            }
        }
        std::pair<uint32_t, uint32_t> v = InnerScore_(toccurs);
        if(v<max)
        {
            max = v;
            max_indexes = indexes;
        }
        if(next_indexes!=indexes)
        {
            //InnerMax_(occurs, next_indexes, max_indexes, max);
        }
    }
    std::pair<bool, double> GetMatchScore2_(const ForwardItem& fi, const std::string& key, const SpuProduct& product, MultiOccurs& occurs) const
    {
        std::pair<bool, double> result(false, 0.0);
        const UString& first_text = fi.word.front().second;
        if(IsSymbol_(first_text)&&occurs.front().empty()) return result;
        const UString& last_text = fi.word.back().second;
        if(IsSymbol_(last_text)&&occurs.back().empty()) return result;
        double min_score = 0.8;
        if(key!="model")
        {
            min_score = 1.0;
        }
        //else if(B5MHelper::IsLooseMatchCategory(product.category))
        //{
        //    min_score = 0.5;
        //}
        else//model in normal categories
        {
            if(fi.word.size()<=3) min_score=1.0;
            else if(fi.word.size()<=5) min_score = 0.8;
            else if(fi.word.size()<=7) min_score = 0.7;
            else min_score = 0.6;
            if(B5MHelper::IsLooseMatchCategory(product.category))
            {
                min_score*=0.8;
            }
            if(min_score<0.5) min_score = 0.5;
        }
#ifdef SM_DEBUG
        std::cerr<<fi.text<<","<<key<<",APPS,"<<min_score;
        for(std::size_t i=0;i<occurs.size();i++)
        {
            std::cerr<<","<<occurs[i].size();
        }
        std::cerr<<std::endl;
#endif
        //static const double a = 0.8;
        //static const double b = 0.2;
        //double e = 0.2;
        std::vector<double> oweights(fi.word.size(), 1.0);
        double osum = 0.0;
        for(std::size_t i=0;i<fi.word.size();i++)
        {
            const UString& text = fi.word[i].second;
            if(text.isAlphaChar(0) || text.isDigitChar(0))
            {
                if(text.length()>=3) oweights[i] = 2.0;
            }
            osum += oweights[i];
        }
        double threshold = 0.0;
        double sum_weight = 0.0;
        for(std::size_t i=0;i<oweights.size();i++)
        {
            double tmp = oweights[i]/osum;
            if(oweights[i]>1.0)
            {
                if(tmp<threshold) threshold = tmp;
            }
            else
            {
                double tmp2 = tmp*2;
                if(tmp2<threshold) threshold = tmp2;
            }
            oweights[i] = tmp;
            sum_weight+=tmp;
        }
        std::pair<double, std::size_t> expect(0.0, 0);
        for(std::size_t i=0;i<occurs.size();i++)
        {
            const std::vector<uint32_t>& tposes = occurs[i];
            expect.second += tposes.size();
            for(std::size_t j=0;j<tposes.size();j++)
            {
                expect.first += ((double)tposes[j]-i);
            }
        }
        expect.first /= expect.second;
        Occurs toccurs(occurs.size(), 0);
        for(std::size_t i=0;i<occurs.size();i++)
        {
            std::vector<uint32_t>& tposes = occurs[i];
            if(tposes.size()==0) continue;
            else if(tposes.size()==1)
            {
                toccurs[i] = tposes.front();
            }
            double iexpect = expect.first+i-0.0001;
            std::pair<double, std::size_t> mindex(9999.9, 0);
            for(std::size_t j=0;j<tposes.size();j++)
            {
                double idist = std::abs(iexpect - tposes[j]);
                if(idist<mindex.first)
                {
                    mindex.first = idist;
                    mindex.second = j;
                }
            }
            toccurs[i] = tposes[mindex.second];
            tposes[0] = toccurs[i];
            tposes.resize(1);
        }
#ifdef SM_DEBUG
        std::cerr<<"[TOCCURS]";
        for(std::size_t i=0;i<toccurs.size();i++)
        {
            std::cerr<<","<<toccurs[i];
        }
        std::cerr<<std::endl;
#endif
        word_t terms;
        terms.reserve(toccurs.size());
        double weight = 0.0;
        double min_weight = 0.0;
        std::size_t ncount=0;
        double ascore = 0.0;
        std::pair<uint32_t, uint32_t> range(0, 0);
        for(std::size_t i=0;i<toccurs.size();i++)
        {
            bool bend = false;
            if(toccurs[i]==0)
            {
                bend = true;
            }
            else if(!terms.empty())
            {
                if(toccurs[i]!=terms.back()+1)
                {
                    bend = true;
                }
            }
            if(bend&&!terms.empty())
            {
                if(weight>=threshold)
                {
                    ascore+=weight;
                    ncount++;
                    if(min_weight==0.0||weight<min_weight) min_weight = weight;
                }
                weight = 0.0;
                terms.resize(0);
            }
            if(toccurs[i]!=0)
            {
                terms.push_back(toccurs[i]);
                weight += oweights[i];
                if(range.first==0)
                {
                    range.first=toccurs[i];
                    range.second=toccurs[i];
                }
                else
                {
                    if(toccurs[i]<range.first) range.first = toccurs[i];
                    if(toccurs[i]>range.second) range.second = toccurs[i];
                }
            }
        }
        if(weight>=threshold)
        {
            ascore+=weight;
            ncount++;
            if(min_weight==0.0||weight<min_weight) min_weight = weight;
        }
        //double mscore = 1.0/(1.0+std::pow(M_E, -1.0*min_weight));// (0, 1)
        double mscore = min_weight/sum_weight;//[0,1]
        mscore = mscore*0.01+0.99;//[0.99,1]
        double bscore = 0.0;
        //if(ncount>0) bscore = 1.0/ncount;
        if(ncount>0)
        {
            //bscore = 1.0/(1.0+std::pow(M_E, -1.0*(ncount-1)));// [0.5, 1)
            //bscore = 1.0-bscore; //(0, 0.5]
            //bscore *= 2; //(0, 1]
            bscore = 1.0/std::pow(1.1, (double)(ncount-1));
        }
        uint32_t edist = range.second-range.first;
        if(edist<fi.word.size())
        {
            edist = 0;
        }
        else
        {
            edist = edist-fi.word.size()+1;
        }
        //double escore = 1.0/(1.0+std::pow(M_E, -1.0*edist));// [0.5, 1)
        //escore = 1.0-escore; //(0, 0.5]
        //escore *= 2; //(0, 1]
        double escore = 1.0/std::pow(1.05, (double)(edist));

        //double score = ascore*a + bscore*b + escore*e;
        //double score = escore*(ascore*a + bscore*b);
        double score = escore*bscore*ascore*mscore;
#ifdef SM_DEBUG
        if(score>=0.0)
        {
            std::cerr<<fi.text<<","<<key<<","<<score<<"("<<ascore<<","<<bscore<<","<<escore<<")"<<std::endl;
        }
#endif
        if(score>=min_score) result.first = true;
        //if(result.first==true&&key=="model")
        //{
        //    LOG(INFO)<<ascore<<","<<bscore<<","<<escore<<","<<mscore<<"\t"<<product.title<<std::endl;
        //}
        result.second = score*std::log(1.0+fi.word.size());
        return result;;
    }
    double GetMatchScore_(const ForwardItem& fi, const std::string& key, const SpuProduct& product, MultiOccurs& occurs) const
    {
        //return GetMatchScore2_(fi, key, product, occurs);
        const UString& first_text = fi.word.front().second;
        if(IsSymbol_(first_text)&&occurs.front().empty()) return 0.0;
        const UString& last_text = fi.word.back().second;
        if(IsSymbol_(last_text)&&occurs.back().empty()) return 0.0;
        //std::vector<std::size_t> max_indexes(occurs.size(), 0);
        //std::vector<std::size_t> indexes(occurs.size(), 0);
        //std::pair<uint32_t, uint32_t> max(100, 100);
        double pdist_ratio = 1.5;
        double poccur_ratio = 0.8;
        double swap_ratio = 0.99;
        if(key=="distrib")
        {
            pdist_ratio = 1.0;
            poccur_ratio = 1.0;
            swap_ratio = 0.0;
        }
        else if(B5MHelper::IsLooseMatchCategory(product.category))
        {
            pdist_ratio = 3.0;
            poccur_ratio = 0.5;
            swap_ratio = 3.0;
        }
#ifdef SM_DEBUG
        //std::cerr<<fi.text<<","<<key<<",pcategory,"<<product.category<<std::endl;
#endif

        std::pair<double, std::size_t> expect(0.0, 0);
        for(std::size_t i=0;i<occurs.size();i++)
        {
            const std::vector<uint32_t>& tposes = occurs[i];
            expect.second += tposes.size();
            for(std::size_t j=0;j<tposes.size();j++)
            {
                expect.first += ((double)tposes[j]-i);
            }
            //if(tposes.empty()) continue;
            //std::size_t index = indexes[i];
            //toccurs[i] = tposes[index];
            //if(!next_set)
            //{
            //    if(index<tposes.size()-1)
            //    {
            //        next_indexes[i] = index+1;
            //        next_set = true;
            //    }
            //    else
            //    {
            //        next_indexes[i] = index;
            //    }
            //}
            //else
            //{
            //    next_indexes[i] = index;
            //}
        }
        expect.first /= expect.second;
        Occurs toccurs(occurs.size(), 0);
        for(std::size_t i=0;i<occurs.size();i++)
        {
            std::vector<uint32_t>& tposes = occurs[i];
            if(tposes.size()==0) continue;
            else if(tposes.size()==1)
            {
                toccurs[i] = tposes.front();
            }
            double iexpect = expect.first+i-0.0001;
            std::pair<double, std::size_t> mindex(9999.9, 0);
            for(std::size_t j=0;j<tposes.size();j++)
            {
                double idist = std::abs(iexpect - tposes[j]);
                if(idist<mindex.first)
                {
                    mindex.first = idist;
                    mindex.second = j;
                }
            }
            toccurs[i] = tposes[mindex.second];
            tposes[0] = toccurs[i];
            tposes.resize(1);
        }
        std::pair<uint32_t, uint32_t> max = InnerScore_(toccurs);
        //InnerMax_(occurs, indexes, max_indexes, max);
        //std::pair<uint32_t, uint32_t> tpos(occurs[0],occurs[0]);
        uint32_t klen = 0;
        for(std::size_t i=0;i<occurs.size();i++)
        {
            if(!occurs[i].empty()) klen++;
            //if(occurs[i]<tpos.first) tpos.first = occurs[i];
            //if(occurs[i]>tpos.second) tpos.second = occurs[i];
        }
        uint32_t dist = max.second;
        double pdist = (double)dist/occurs.size();
#ifdef SM_DEBUG
        std::cerr<<fi.text<<","<<key<<",pdist,"<<pdist<<std::endl;
#endif
        if(pdist>pdist_ratio) return 0.0;
        //if(key=="capacity"&&occurs.size()==2&&occurs[0]>0) return 0.8;
        double poccur = (double)klen/occurs.size();
#ifdef SM_DEBUG
        std::cerr<<fi.text<<","<<key<<",poccur,"<<poccur<<std::endl;
#endif
        if(poccur<poccur_ratio) return 0.0;
        uint32_t iswap = max.first;
        //uint32_t iswap = 0;
        //Occurs kpos(occurs);
        //for(uint32_t i=0;i<kpos.size();i++)
        //{
        //    if(kpos[i]==0) continue;
        //    for(uint32_t j=i;j<kpos.size()-1;j++)
        //    {
        //        if(kpos[j]==0) continue;
        //        if(kpos[j]>kpos[j+1])
        //        {
        //            std::swap(kpos[j], kpos[j+1]);
        //            iswap++;
        //        }
        //    }
        //}
        double swap = (double)iswap*2/occurs.size();
#ifdef SM_DEBUG
        std::cerr<<fi.text<<","<<key<<",swap,"<<swap<<std::endl;
#endif
        if(swap>swap_ratio) return 0.0;
        return poccur*fi.word.size();
    }
    uint32_t GetTerm_(const std::string& str)
    {
        uint32_t term = izenelib::util::HashFunction<std::string>::generateHash32(str);
        return term;
    }
    void GetWordS_(const std::string& text, word_t& word)
    {
        UString title(text, UString::UTF_8);
        std::vector<idmlib::util::IDMTerm> terms;
        analyzer_->GetTermList(title, terms);
        for(uint32_t i=0;i<terms.size();i++)
        {
            std::string str = terms[i].TextString();
            boost::algorithm::to_lower(str);
            word.push_back(GetTerm_(str));
        }
    }
    void GetWordS_(const std::string& text, Word& word, word_t& iword)
    {
        UString title(text, UString::UTF_8);
        std::vector<idmlib::util::IDMTerm> terms;
        analyzer_->GetTermList(title, terms);
        for(uint32_t i=0;i<terms.size();i++)
        {
            std::string str = terms[i].TextString();
            std::string lstr = boost::algorithm::to_lower_copy(str);
            uint32_t term = GetTerm_(lstr);
            word.push_back(std::make_pair(term, terms[i].text));
            iword.push_back(term);
        }
    }
    bool IsParent_(uint32_t cidx, uint32_t cidy) const
    {
        uint32_t cid = cidy;
        while(true)
        {
            if(cid==cidx) return true;
            if(cid==0) break;
            cid = cm_.data[cid].parent_cid;
        }
        return false;
    }
    void LoadAccessoryBrand_(const std::string& abfile)
    {
        std::ifstream ifs(abfile.c_str());
        std::string line;
        while( getline(ifs, line))
        {
            boost::algorithm::trim(line);
            UString uline(line, UString::UTF_8);
            if(uline.length()<2) continue;
            word_t word;
            GetWordS_(line, word);
            adic_.insert(word);
        }
        ifs.close();
    }
private:
    idmlib::util::IDMAnalyzer* analyzer_;
    CategoryManager cm_;
    std::vector<SpuProduct> products_;
    Forward forward_;
    Invert invert_;
    ForwardIndex forward_index_;
    boost::shared_ptr<msgpack::rpc::client> be_;
    AccessoryDic adic_;
#ifdef SM_DEBUG
    std::size_t debug_;
#endif
};

NS_IDMLIB_B5M_END

#endif

