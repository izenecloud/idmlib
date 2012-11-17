#ifndef IDMLIB_SIM_STRINGSIMILARITY_H_
#define IDMLIB_SIM_STRINGSIMILARITY_H_


#include <string>
#include <iostream>
#include <idmlib/idm_types.h>
#include <glog/logging.h>
#include <idmlib/util/idm_analyzer.h>
NS_IDMLIB_SIM_BEGIN

//#define SS_DEBUG


class StringSimilarity
{
typedef izenelib::util::UString UString;
typedef izenelib::util::UCS2Char UChar;

public:

    typedef uint32_t TermType;


    class SuffixType
    {
        typedef std::vector<TermType> TermList;
        typedef std::vector<TermList> Suffixes;
    public:
        struct Node
        {
            typedef Suffixes::const_iterator iterator;
            
            std::pair<iterator, iterator> range;
            uint32_t len;
        };
        SuffixType()
        {
        }
        SuffixType(const std::vector<TermType>& terms)
        {
            SetTerms(terms);
        }
        void SetTerms(const std::vector<TermType>& terms)
        {
            suffixes.resize(0);
            for(uint32_t i=0;i<terms.size();i++)
            {
                TermList suffix(terms.begin()+terms.size()-i-1, terms.end());
                suffixes.push_back(suffix);
            }
            std::sort(suffixes.begin(), suffixes.end());
        }

        bool Search(const Node& parent_node, const TermType& term, Node& node) const
        {
            //LOG(INFO)<<"suffixes size "<<suffixes.size()<<std::endl;
            Suffixes::const_iterator start = parent_node.range.first;
            if(start!=suffixes.end())
            {
                TermList term_list(parent_node.len+1, term);
                for(uint32_t i=0;i<parent_node.len;i++)
                {
                    term_list[i] = (*start)[i];
                }
#ifdef SS_DEBUG
                LOG(INFO)<<"parent range "<<parent_node.range.first-suffixes.begin()<<","<<parent_node.range.second-suffixes.begin()<<std::endl;
#endif
                std::pair<Node::iterator, Node::iterator> bounds(suffixes.end(), suffixes.end());
                //bounds = std::equal_range(parent_node.range.first, parent_node.range.second, term_list);
                Node::iterator lit = std::lower_bound(parent_node.range.first, parent_node.range.second, term_list);
                for(Suffixes::const_iterator it = lit; it!=parent_node.range.second;++it)
                {
                    if(StartsWith_(*it, term_list))
                    {
                        if(bounds.first==suffixes.end())
                        {
                            bounds.first = it;
                        }
                    }
                    else
                    {
                        if(bounds.first!=suffixes.end())
                        {
                            bounds.second = it;
                            break;
                        }
                    }
                }
#ifdef SS_DEBUG
                LOG(INFO)<<"result range "<<bounds.first-suffixes.begin()<<","<<bounds.second-suffixes.begin()<<std::endl;
#endif
                if(bounds.first>=bounds.second)
                {
#ifdef SS_DEBUG
                    LOG(INFO)<<"equal range empty"<<std::endl;
#endif
                    return false;
                }
                node.range = bounds;
                node.len = parent_node.len+1;
#ifdef SS_DEBUG
                LOG(INFO)<<"find range "<<node.range.first-suffixes.begin()<<","<<node.range.second-suffixes.begin()<<std::endl;
#endif
                return true;
            }
            else
            {
                //LOG(INFO)<<"start=end"<<std::endl;
                return false;
            }
        }

        Node GetTopNode() const
        {
            Node node;
            node.range = std::make_pair(suffixes.begin(), suffixes.end());
            node.len = 0;
            return node;
        }

    private:

        static bool StartsWith_(const TermList& t1, const TermList& t2)
        {
            if(t1.size()<t2.size()) return false;
            for(uint32_t i=0;i<t2.size();i++)
            {
                if(t2[i]!=t1[i]) return false;
            }
            return true;
        }

        //bool Compare_(uint32_t len, const TermType& term, const TermList& x, const TermList& y)
        //{
            //if(len>=x.size())
            //{
                //return true;
            //}
            //else if(len>=y.size())
            //{
                //return false;
            //}
            //else if(x[len]<term&&y[len])
            //{
                //return x[len]<y[len];
            //}
            //else
            //{
                //return false;
            //}
        //}

    public:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version)
        {
            ar & suffixes;
        }

        Suffixes suffixes;
    };



    struct Object
    {
        UString text;
        std::vector<TermType> terms;
        SuffixType suffix;
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version)
        {
            ar & text & terms & suffix;
        }
    };

public:
    StringSimilarity()
    {
        idmlib::util::IDMAnalyzerConfig config = idmlib::util::IDMAnalyzerConfig::GetCommonConfig("","", "");
        analyzer_ = new idmlib::util::IDMAnalyzer(config);
    }

    ~StringSimilarity()
    {
        delete analyzer_;
    }

    void Convert(const UString& text, Object& obj)
    {
        //obj.text = text;
        std::vector<UString> str_list;
        analyzer_->GetStringList(text, str_list);
        std::vector<TermType> id_list(str_list.size());
        for(uint32_t i=0;i<str_list.size();i++)
        {
            id_list[i] = izenelib::util::HashFunction<UString>::generateHash32(str_list[i]);
        }
        obj.terms.swap(id_list);
        obj.suffix.SetTerms(obj.terms);
    }

    void Convert(const std::string& text, Object& obj)
    {
        UString utext(text, UString::UTF_8);
        Convert(utext, obj);
    }

    //double Sim(const UString& y)
    //{
        //static double length_weight = 0.8;
        //uint32_t match_length = 0;
        //uint32_t match_count = 0;
        //std::size_t iy = 0;
        //while(true)
        //{
            //if(iy>=y.length()) break;
            //UString m;
            //GetMatch_(y, iy, m);
            //if(!m.empty())
            //{
                //match_length += m.length();
                //match_count++;
                //iy += m.length();
                //std::string sm;
                //m.convertString(sm, UString::UTF_8);
                //LOG(INFO)<<"[SMATCH]"<<sm<<std::endl;
            //}
            //else
            //{
                //iy++;
            //}
        //}
        //double sim = 0.0;
        //if(match_count>0)
        //{
            //sim = (double)match_length/y.length()*length_weight + 1.0/match_count*(1.0-length_weight);
        //}
        //return sim;
    //}

    double Sim(const UString& x, const UString& y)
    {
        //StringSimilarity ssx(x);
        //double sx = ssx.Sim(y);
        //StringSimilarity ssy(y);
        //double sy = ssy.Sim(x);
        //return 0.5*sx+0.5*sy;

        Object ox, oy;
        Convert(x, ox);
        Convert(y, oy);
        return Sim(ox, oy);
    }

    double Sim(const std::string& x, const std::string& y)
    {
        return Sim(UString(x, UString::UTF_8), UString(y, UString::UTF_8));
    }

    static double SimBaise(const Object& x, const Object& y)
    {
        return GetSim_(x.terms, y.suffix);
    }

    static double Sim(const Object& x, const Object& y)
    {
        return GetSim_(x.terms, y.suffix)*0.5+GetSim_(y.terms, x.suffix)*0.5;
    }

private:

    static double GetSim_(const std::vector<TermType>& terms, const SuffixType& suffix)
    {
#ifdef SS_DEBUG
        LOG(INFO)<<"terms length "<<terms.size()<<std::endl;
        LOG(INFO)<<"suffixes length "<<suffix.suffixes.size()<<std::endl;
        for(uint32_t i=0;i<suffix.suffixes.size();i++)
        {
            const std::vector<TermType>& tl = suffix.suffixes[i];
            std::cerr<<"suffix "<<i<<": ";
            for(uint32_t j=0;j<tl.size();j++)
            {
                std::cerr<<tl[j]<<",";
            }
            std::cerr<<std::endl;
        }
#endif
        static double length_weight = 0.95;
        uint32_t match_length = 0;
        uint32_t match_count = 0;
        uint32_t c_match_length = 0;
        SuffixType::Node top_node = suffix.GetTopNode();
        SuffixType::Node node = top_node;
        //uint32_t begin_position = 0;
        for(uint32_t i=0;i<terms.size();i++)
        {
            const TermType& term = terms[i];
            SuffixType::Node child;
            if(suffix.Search(node, term, child))
            {
#ifdef SS_DEBUG
                LOG(INFO)<<"term "<<i<<","<<term<<" found"<<std::endl;
#endif
                match_length++;
                c_match_length++;
            }
            else
            {
#ifdef SS_DEBUG
                LOG(INFO)<<"term "<<i<<","<<term<<" not found"<<std::endl;
#endif
                if(c_match_length>0)
                {
                    match_count++;
                    c_match_length = 0;
                }
                node = top_node;
            }
        }
        if(c_match_length>0)
        {
            match_count++;
        }
        double sim = 0.0;
        if(match_count>0)
        {
            //LOG(INFO)<<"[P]"<<match_length<<","<<terms.size()<<","<<match_count<<std::endl;
            sim = (double)match_length/terms.size()*length_weight + 1.0/match_count*(1.0-length_weight);
            //sim = (double)match_length/terms.size()/log2_(match_count+1.0);
            //sim = (double)match_length/terms.size();
        }
        return sim;
    }

    static double log2_(double v)
    {
        return std::log(v)/std::log(2.0);
    }

    //void GetMatch_(const UString& y, std::size_t iy, UString& m)
    //{
        //for(std::size_t ix = 0;ix<x_.length();ix++)
        //{
            //UString im;
            //GetMatch_(ix, y, iy, im);
            //if(!im.empty())
            //{
                //if(im.length()>m.length())
                //{
                    //m = im;
                //}
            //}
        //}

    //}

    //void GetMatch_(std::size_t ix, const UString& y, std::size_t iy, UString& m)
    //{
        //if(ix>=x_.length() || iy>=y.length()) return;
        //if(x_[ix]==y[iy])
        //{
            //m += x_[ix];
            //GetMatch_(ix+1, y, iy+1, m);
        //}
    //}

private:
    idmlib::util::IDMAnalyzer* analyzer_;
  
};

   
NS_IDMLIB_SIM_END

#endif 

