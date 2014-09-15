#ifndef IDMLIB_B5M_B5MHELPER_H_
#define IDMLIB_B5M_B5MHELPER_H_
#include "b5m_types.h"
#include <string>
#include <vector>
#include <boost/assign/list_of.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <sf1common/ScdParser.h>
#include <sf1common/Utilities.h>
#include <sf1common/split_ustr.h>
#include <types.h>

NS_IDMLIB_B5M_BEGIN

class CategoryStringMatcher
{
public:
    CategoryStringMatcher(bool reverse = false):reverse_(reverse)
    {
    }
    void AddCategoryRegex(const std::string& r)
    {
        boost::regex reg(r);
        regex_list_.push_back(reg);
    }
    bool Match(const std::string& scategory) const
    {
        for(uint32_t i=0;i<regex_list_.size();i++)
        {
            if(boost::regex_match(scategory, regex_list_[i]))
            {
                return !reverse_;
            }

        }
        return reverse_;
    }
private:
    bool reverse_;
    std::vector<boost::regex> regex_list_;
};

class B5MHelper {
public:

    static uint128_t StringToUint128(const std::string& str)
    {
        unsigned long long high = 0, low = 0;
        sscanf(str.c_str(), "%016llx%016llx", &high, &low);
        return (uint128_t) high << 64 | (uint128_t) low;
    }

    static uint128_t UStringToUint128(const izenelib::util::UString& ustr)
    {
        std::string str;
        ustr.convertString(str, izenelib::util::UString::UTF_8);
        return StringToUint128(str);
    }

    static std::string Uint128ToString(const uint128_t& val)
    {
        char tmpstr[33];
        sprintf(tmpstr, "%016llx%016llx", (unsigned long long) (val >> 64), (unsigned long long) val);
        return std::string(reinterpret_cast<const char *>(tmpstr), 32);
    }

    static int64_t CommentCountToSalesAmount(int64_t cmt)
    {
        double r = 0.73*cmt - 0.00001*8.782*cmt*cmt+0.000000001*3.242*cmt*cmt*cmt-0.53;
        return std::max(0l, (int64_t)r);
    }

    static void GetScdList(const std::string& scd_path, std::vector<std::string>& scd_list)
    {
        namespace bfs = boost::filesystem;
        if(!bfs::exists(scd_path)) return;
        if( bfs::is_regular_file(scd_path) && boost::algorithm::ends_with(scd_path, ".SCD"))
        {
            scd_list.push_back(scd_path);
        }
        else if(bfs::is_directory(scd_path))
        {
            bfs::path p(scd_path);
            bfs::directory_iterator end;
            for(bfs::directory_iterator it(p);it!=end;it++)
            {
                if(bfs::is_regular_file(it->path()))
                {
                    std::string file = it->path().string();
                    if(ScdParser::checkSCDFormat(file))
                    {
                        scd_list.push_back(file);
                    }
                }
            }
        }
        std::sort(scd_list.begin(), scd_list.end(), ScdParser::compareSCD);
    }

    static void GetIScdList(const std::string& scd_path, std::vector<std::string>& scd_list)
    {
        GetScdList(scd_path, scd_list);
        std::vector<std::string> valid_scd_list;
        for(uint32_t i=0;i<scd_list.size();i++)
        {
            int scd_type = ScdParser::checkSCDType(scd_list[i]);
            if(scd_type==INSERT_SCD)
            {
                valid_scd_list.push_back(scd_list[i]);
            }
        }
        scd_list.swap(valid_scd_list);
    }

    static void GetIUScdList(const std::string& scd_path, std::vector<std::string>& scd_list)
    {
        GetScdList(scd_path, scd_list);
        std::vector<std::string> valid_scd_list;
        for(uint32_t i=0;i<scd_list.size();i++)
        {
            int scd_type = ScdParser::checkSCDType(scd_list[i]);
            if(scd_type!=DELETE_SCD)
            {
                valid_scd_list.push_back(scd_list[i]);
            }
        }
        scd_list.swap(valid_scd_list);
    }

    static void PrepareEmptyDir(const std::string& dir)
    {
        boost::filesystem::remove_all(dir);
        boost::filesystem::create_directories(dir);
    }

	static bool RemoveDir(const std::string &dir)
	{
		try 
		{
			boost::filesystem::remove_all(dir);
			return true;
		} 
		catch (boost::filesystem::filesystem_error &e)
		{	
			return false;
		}
	}

	static bool CopyFile(const boost::filesystem::path &src, 
				         const boost::filesystem::path &dst)
	{
		try
		{
			if (!boost::filesystem::exists(dst))
			{
				boost::filesystem::create_directories(dst);
			}
			for (boost::filesystem::directory_iterator it(src); it != 
					boost::filesystem::directory_iterator(); ++it)
			{
				const boost::filesystem::path new_src = it->path();
				const boost::filesystem::path new_dst = dst / new_src.filename();
				if (boost::filesystem::is_directory(new_src))
				{
					CopyFile(new_src, new_dst);
				}
				else if (boost::filesystem::is_regular_file(new_src))
				{
					boost::filesystem::copy_file(new_src, new_dst, 
						boost::filesystem::copy_option::overwrite_if_exists);
				}
			}
			return true;
		} 
		catch(boost::filesystem::filesystem_error &e)
		{
			return false;
		}
	} 

    static void SplitAttributeValue(const std::string& str, std::vector<std::string>& str_list)
    {
        boost::algorithm::split(str_list, str, boost::algorithm::is_any_of("/"));
    }

    static std::string GetOdbPath(const std::string& mdb_instance)
    {
        return mdb_instance+"/odb";
    }
    static std::string GetRawPath(const std::string& mdb_instance)
    {
        return mdb_instance+"/raw";
    }
    static std::string GetB5moMirrorPath(const std::string& mdb_instance)
    {
        return mdb_instance+"/b5mo_mirror";
    }
    static std::string GetB5moPath(const std::string& mdb_instance)
    {
        return mdb_instance+"/b5mo";
    }
    static std::string GetB5moBlockPath(const std::string& mdb_instance)
    {
        return mdb_instance+"/b5mo_block";
    }
    static std::string GetB5mpPath(const std::string& mdb_instance)
    {
        return mdb_instance+"/b5mp";
    }
    static std::string GetUuePath(const std::string& mdb_instance)
    {
        return mdb_instance+"/uue";
    }
    static std::string GetB5mcPath(const std::string& mdb_instance)
    {
        return mdb_instance+"/b5mc";
    }
    static std::string GetTmpPath(const std::string& mdb_instance)
    {
        return mdb_instance+"/tmp";
    }
    static std::string GetPoMapPath(const std::string& mdb_instance)
    {
        return mdb_instance+"/po_map";
    }

    static std::string GetOMapperPath(const std::string& mdb_instance)
    {
        return mdb_instance+"/omapper";
    }

    static std::string GetPsmPath(const std::string& mdb_instance)
    {
        return mdb_instance+"/psm";
    }

    static std::string GetBrandPropertyName()
    {
        static std::string p("Brand");
        return p;
    }

    static std::string GetSubPropPropertyName()
    {
        static std::string p("SubProp");
        return p;
    }
    static std::string GetSubPropsPropertyName()
    {
        static std::string p("SubProps");
        return p;
    }

    static std::string GetSPTPropertyName()
    {
        static std::string p("SPTitle");
        return p;
    }
    static std::string GetSPPicPropertyName()
    {
        static std::string p("SPPicture");
        return p;
    }
    static std::string GetSPUrlPropertyName()
    {
        static std::string p("SPUrl");
        return p;
    }
    static std::string GetTargetCategoryPropertyName()
    {
        static std::string p("TargetCategory");
        return p;
    }
    static std::string GetSubDocsPropertyName()
    {
        static std::string p("SubDocs");
        return p;
    }

    static std::string GetSalesAmountPropertyName()
    {
        static std::string p("SalesAmount");
        return p;
    }

    static std::string GetCommentCountPropertyName()
    {
        static std::string p("CommentCount");
        return p;
    }
    static std::string GetProductTypePropertyName()
    {
        static std::string p("ProductType");
        return p;
    }
    static std::string GetTagsPropertyName()
    {
        static std::string p("Tags");
        return p;
    }
    static std::string GetOEquiCountPropertyName()
    {
        static std::string p("EquiCount");
        return p;
    }
    static std::string GetIsLowCompPricePropertyName()
    {
        static std::string p("isLowCompPrice");
        return p;
    }
    static std::string RefPricePN()
    {
        static std::string p("RefPrice");
        return p;
    }
    static std::string RawRankPN()
    {
        static std::string p("RawRank");
        return p;
    }

    static std::string BookCategoryName()
    {
        static std::string name("书籍/杂志/报纸");
        return name;
    }

    static bool Is3cAccessoryCategory(const std::string& category)
    {
        if(boost::algorithm::starts_with(category, "手机数码>手机数码配件") || boost::algorithm::starts_with(category, "手机数码>相机配件"))
        {
            return true;
        }
        else return false;
    }
    static bool IsLooseMatchCategory(const std::string& category)
    {
        if(boost::algorithm::starts_with(category, "手机数码>手机数码配件") || boost::algorithm::starts_with(category, "手机数码>相机配件") || boost::algorithm::starts_with(category, "电脑办公>电脑外设"))
        {
            return true;
        }
        else return false;
    }

    static std::string GetPidByIsbn(const std::string& isbn)
    {
        //static const int MD5_DIGEST_LENGTH = 32;
        std::string url = "http://www.b5m.com/spuid/isbn/"+isbn;
        return izenelib::Utilities::generateMD5(url);

        //md5_state_t st;
        //md5_init(&st);
        //md5_append(&st, (const md5_byte_t*)(url.c_str()), url.size());
        //union
        //{
            //md5_byte_t digest[MD5_DIGEST_LENGTH];
            //uint128_t md5_int_value;
        //} digest_union;			
        //memset(digest_union.digest, 0, sizeof(digest_union.digest));
        //md5_finish(&st,digest_union.digest);

        ////uint128_t pid = izenelib::util::HashFunction<UString>::generateHash128(UString(pid_str, UString::UTF_8));
        //return B5MHelper::Uint128ToString(digest_union.md5_int_value);
    }

    static std::string GetPidByUrl(const std::string& url)
    {
        return izenelib::Utilities::generateMD5(url);
    }

    static void ParseAttributes(const izenelib::util::UString& ustr, std::vector<b5m::Attribute>& attributes)
    {
        typedef izenelib::util::UString UString;
        std::vector<izenelib::AttrPair> attrib_list;
        std::vector<std::pair<UString, std::vector<UString> > > my_attrib_list;
        izenelib::split_attr_pair(ustr, attrib_list);
        for(std::size_t i=0;i<attrib_list.size();i++)
        {
            Attribute attribute;
            attribute.is_optional = false;
            const std::vector<izenelib::util::UString>& attrib_value_list = attrib_list[i].second;
            if(attrib_value_list.empty()) continue;
            izenelib::util::UString attrib_value = attrib_value_list[0];
            for(uint32_t a=1;a<attrib_value_list.size();a++)
            {
                attrib_value.append(UString(" ",UString::UTF_8));
                attrib_value.append(attrib_value_list[a]);
            }
            if(attrib_value.empty()) continue;
            //if(attrib_value_list.size()!=1) continue; //ignore empty value attrib and multi value attribs
            izenelib::util::UString attrib_name = attrib_list[i].first;
            //if(attrib_value.length()==0 || attrib_value.length()>30) continue;
            attrib_name.convertString(attribute.name, UString::UTF_8);
            boost::algorithm::trim(attribute.name);
            if(boost::algorithm::ends_with(attribute.name, "_optional"))
            {
                attribute.is_optional = true;
                attribute.name = attribute.name.substr(0, attribute.name.find("_optional"));
            }
            std::vector<std::string> value_list;
            std::string svalue;
            attrib_value.convertString(svalue, izenelib::util::UString::UTF_8);
            boost::algorithm::split(attribute.values, svalue, boost::algorithm::is_any_of("/"));
            for(uint32_t v=0;v<attribute.values.size();v++)
            {
                boost::algorithm::trim(attribute.values[v]);
            }
            //if(attribute.name=="容量" && attribute.values.size()==1 && boost::algorithm::ends_with(attribute.values[0], "G"))
            //{
                //std::string gb_value = attribute.values[0]+"B";
                //attribute.values.push_back(gb_value);
            //}
            attributes.push_back(attribute);
        }
    }

    static void LoadCategories(const std::string& file, CategoryManager& cm) 
    {
        std::string line;
        std::ifstream ifs(file.c_str());
        cm.data.resize(1);
        cm.data[0].is_parent = true;
        cm.data[0].depth = 0;
        cm.data[0].cid = 0;
        while(getline(ifs, line))
        {
            boost::algorithm::trim(line);
            std::vector<std::string> vec;
            boost::algorithm::split(vec, line, boost::algorithm::is_any_of(","));
            if(vec.size()<1) continue;
            const std::string& scategory = vec[0];
            if(scategory.empty()) continue;
            uint32_t cid = cm.data.size();
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
                c.parent_cid = cm.index[parent_name];
                //std::cerr<<"cid "<<c.cid<<std::endl;
                //std::cerr<<"parent cid "<<c.parent_cid<<std::endl;
            }
            else
            {
                c.parent_cid = 0;
            }
            Category& pc = cm.data[c.parent_cid];
            pc.is_parent = true;
            pc.children.push_back(cid);
            cm.index[scategory] = cid;
            cm.data.push_back(c);
        }
        ifs.close();
    }

    static void GetAttributeBrand(const Document& doc, std::vector<std::string>& brand)
    {
        UString attribute;
        doc.getString("Attribute", attribute);
        if(attribute.empty()) return;
        std::vector<Attribute> attributes;
        ParseAttributes(attribute, attributes);
        for(std::size_t i=0;i<attributes.size();i++)
        {
            const Attribute& a = attributes[i];
            if(a.name=="品牌")
            {
                brand = a.values;
                return;
            }
        }
    }
    static void GetBrandAndApplyto(const Document& doc, std::vector<std::string>& brand, std::string& applyto)
    {
        static const std::string pbrand("品牌");
        static const std::string papplyto1("适用于");
        static const std::string papplyto2("适用");
        UString attribute;
        doc.getString("Attribute", attribute);
        if(attribute.empty()) return;
        std::vector<Attribute> attributes;
        ParseAttributes(attribute, attributes);
        for(std::size_t i=0;i<attributes.size();i++)
        {
            const Attribute& a = attributes[i];
            if(a.name==pbrand)
            {
                brand = a.values;
            }
            else if(a.name==papplyto1)
            {
                applyto = a.GetValue();
            }
            else if(a.name==papplyto2&&applyto.empty())
            {
                applyto = a.GetValue();
            }
        }
        if(applyto=="其他"||applyto=="其它")
        {
            applyto.clear();
        }
        if(applyto.empty())
        {
            std::string title;
            doc.getString("Title", title);
            std::size_t len = papplyto1.length();
            std::size_t pos = title.find(papplyto1);
            if(pos==std::string::npos)
            {
                pos = title.find(papplyto2);
                len = papplyto2.length();
            }
            if(pos!=std::string::npos)
            {
                std::string sleft = title.substr(pos+len);
                UString left(sleft, UString::UTF_8);
                std::size_t ulen = left.length()>4 ? 4 : left.length();
                left = left.substr(0, ulen);
                left.convertString(applyto, UString::UTF_8);
            }
        }
    }

};

NS_IDMLIB_B5M_END

#endif

