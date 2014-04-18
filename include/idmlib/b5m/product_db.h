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
public:
    B5mpDocGenerator();
    void Gen(const std::vector<ScdDocument>& odocs, ScdDocument& pdoc);
private:
    void SelectSubDocs_(std::vector<Document>& subdocs) const;
    static bool SubDocCompare_(const Document& x, const Document& y);
    static bool ReversePriceCompare_(const Document& x, const Document& y);

private:
    boost::unordered_set<std::string> sub_doc_props_;
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

