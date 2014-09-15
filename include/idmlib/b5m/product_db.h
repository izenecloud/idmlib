#ifndef IDMLIB_B5M_PRODUCTDB_H_
#define IDMLIB_B5M_PRODUCTDB_H_

#include "b5m_types.h"
#include <string>
#include <vector>
#include "product_price.h"
#include <sf1common/Document.h>
#include <sf1common/ScdDocument.h>
#include <glog/logging.h>
#include <boost/unordered_set.hpp>

NS_IDMLIB_B5M_BEGIN

struct SubDocSelector
{
    int weight;
    std::vector<Document> docs;
    bool operator<(const SubDocSelector& x) const
    {
        return weight>x.weight;
    }

};

class B5mpDocGenerator
{
    struct SubPropsValue
    {
        SubPropsValue() : price(0.0)
        {
        }
        SubPropsValue(const std::string& pname): name(pname), price(0.0)
        {
        }
        std::string name;
        double price;
        std::vector<Document> docs;
        static bool SizeCompare(const SubPropsValue& x, const SubPropsValue& y)
        {
            return x.docs.size()>y.docs.size();
        }
        static bool PriceCompare(const SubPropsValue& x, const SubPropsValue& y)
        {
            return x.price>y.price;
        }
    };

    typedef boost::unordered_map<std::string, SubPropsValue> SubPropsMap;
    typedef std::vector<SubPropsValue> SubProps;
public:
    B5mpDocGenerator();
    void Gen(const std::vector<ScdDocument>& odocs, ScdDocument& pdoc);
private:
    void SelectSubDocs_(std::vector<Document>& subdocs) const;
    static bool DOCIDCompare_(const Document& x, const Document& y);
    static bool SubDocCompare_(const Document& x, const Document& y);
    static bool ReversePriceCompare_(const Document& x, const Document& y);
    void GenSubProps_(const std::vector<ScdDocument>& odocs, SubProps& sp) const;

private:
    boost::unordered_set<std::string> sub_doc_props_;
    boost::unordered_set<std::string> subprops_props_;
    boost::unordered_map<std::string, int> subdoc_weighter_;
    boost::unordered_map<std::string, int> pic_weighter_;
    int default_source_weight_;
};

class ProductProperty
{
public:
    //typedef std::map<std::string, int32_t> SourceMap;
    typedef boost::unordered_set<std::string> SourceType;
    typedef Document::doc_prop_value_strtype StringType;
    typedef std::map<std::string, std::vector<std::string> > AttributeType;

    Document::doc_prop_value_strtype productid;
    ProductPrice price;
    SourceType source;
    int64_t itemcount;
    StringType oid;
    bool independent;
    AttributeType attribute;
    std::string date;

    ProductProperty();

    bool Parse(const Document& doc);

    void Set(Document& doc) const;

    void SetIndependent();


    std::string GetSourceString() const;

    izenelib::util::UString GetSourceUString() const;
    izenelib::util::UString GetAttributeUString() const;

    ProductProperty& operator+=(const ProductProperty& other);

    //ProductProperty& operator-=(const ProductProperty& other);

    std::string ToString() const;
};

NS_IDMLIB_B5M_END

#endif

