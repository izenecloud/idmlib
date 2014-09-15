#ifndef IDMLIB_B5M_B5MM_H_
#define IDMLIB_B5M_B5MM_H_

#include <3rdparty/yaml-cpp/yaml.h>
#include <boost/filesystem.hpp>

NS_IDMLIB_B5M_BEGIN
struct B5mM
{
    B5mM(): mode(0), cmode(-1), thread_num(1), gen_b5ma(false), gen_b5mq(false), rtype(false), gen_tags(false), gen_oextra(false), classifier_port(0), comment_port(0), brand_port(0)
    {
    }
    B5mM(const std::string& m)
    {
        SetDefaultValue_(m);
        Load(m);
    }
    bool Load(const std::string& m)
    {
        SetDefaultValue_(m);
        path = m;
        std::string m_config_file = m+"/config";
        if(!boost::filesystem::exists(m_config_file)) return false;
        YAML::Node config = YAML::LoadFile(m_config_file);
        if(config.Type()==YAML::NodeType::Undefined) return false;
        YAML::Node rconfig = config["config"];
        if(rconfig.Type()==YAML::NodeType::Undefined)
        {
            rconfig = config[":config"];
        }
        if(rconfig.Type()==YAML::NodeType::Undefined)
        {
            rconfig = config;
        }
        config = rconfig;
        if(config.Type()==YAML::NodeType::Undefined) return false;
        YAML::Node path_of = config["path_of"];
        if(path_of.Type()==YAML::NodeType::Undefined) return false;
        YAML::Node indexer = config["indexer"];
        if(indexer.Type()==YAML::NodeType::Undefined) return false;
        SetValue_(path_of["scd"], scd_path);
        SetValue_(path_of["comment_scd"], comment_scd_path);
        SetValue_(path_of["mobile_source"], mobile_source);
        SetValue_(path_of["human_match"], human_match);
        SetValue_(path_of["knowledge"], knowledge);
        SetValue_(path_of["addr_knowledge"], addr_knowledge);
        SetValue_(path_of["tag_knowledge"], tag_knowledge);
        SetValue_(path_of["maxent_title_knowledge"], maxent_title_knowledge);
        SetValue_(path_of["cma"], cma_path);
        SetValue_(path_of["rank_scd"], rank_scd_path);
        if(indexer["type"].as<std::string>()=="hdfs")
        {
            std::string cname = config["schema"].as<std::string>();
            if(indexer["collection_name"].Type()!=YAML::NodeType::Undefined)
            {
                cname = indexer["collection_name"].as<std::string>();
            }
            std::string hdfs_dir = indexer["hdfs_mnt"].as<std::string>()+"/"+indexer["hdfs_prefix"].as<std::string>()+"/"+cname+"/"+ts;
            if(cname=="tuan")
            {
                b5mo_path = hdfs_dir+"/"+cname+"m";
                b5mp_path = hdfs_dir+"/"+cname+"a";
                b5mc_path = hdfs_dir+"/"+cname+"c";
            }
            else
            {
                b5mo_path = hdfs_dir+"/"+cname+"o";
                b5mp_path = hdfs_dir+"/"+cname+"p";
                b5mc_path = hdfs_dir+"/"+cname+"c";
                b5ma_path = hdfs_dir+"/"+cname+"a";
                b5mq_path = hdfs_dir+"/"+cname+"q";
            }
            if(indexer["o_collection_name"].Type()!=YAML::NodeType::Undefined)
            {
                b5mo_path = hdfs_dir+"/"+indexer["o_collection_name"].as<std::string>();
            }
            if(indexer["p_collection_name"].Type()!=YAML::NodeType::Undefined)
            {
                b5mp_path = hdfs_dir+"/"+indexer["p_collection_name"].as<std::string>();
            }
        }
        SetValue_(config["mode"], mode);
        SetValue_(config["cmode"], cmode);
        SetValue_(config["thread_num"], thread_num);
        SetValue_(config["buffer_size"], buffer_size);
        SetValue_(config["sorter_bin"], sorter_bin);
        SetValue_(config["gen_b5ma"], gen_b5ma);
        SetValue_(config["gen_b5mq"], gen_b5mq);
        SetValue_(config["rtype"], rtype);
        SetValue_(config["gen_tags"], gen_tags);
        SetValue_(config["gen_oextra"], gen_oextra);
        std::string classifier_str;
        SetValue_(config["classifier"], classifier_str);
        std::vector<std::string> vec;
        boost::algorithm::split(vec, classifier_str, boost::is_any_of(":"));
        if(vec.size()==2)
        {
            classifier_ip = vec[0];
            classifier_port = boost::lexical_cast<int>(vec[1]);
        }
        std::string comment_str;
        SetValue_(config["comment"], comment_str);
        vec.clear();
        boost::algorithm::split(vec, comment_str, boost::is_any_of(":"));
        if(vec.size()==2)
        {
            comment_ip = vec[0];
            comment_port = boost::lexical_cast<int>(vec[1]);
        }
        std::string brand_str;
        SetValue_(config["brand"], brand_str);
        vec.clear();
        boost::algorithm::split(vec, brand_str, boost::is_any_of(":"));
        if(vec.size()==2)
        {
            brand_ip = vec[0];
            brand_port = boost::lexical_cast<int>(vec[1]);
        }
        return true;
    }
    void Gen()
    {
        SetDefaultValue_(path);
    }
    void Show()
    {
        std::cerr<<"path: "<<path<<std::endl;
        std::cerr<<"ts: "<<ts<<std::endl;
        std::cerr<<"mode: "<<mode<<std::endl;
        std::cerr<<"cmode: "<<cmode<<std::endl;
        std::cerr<<"thread_num: "<<thread_num<<std::endl;
        std::cerr<<"b5mo_path: "<<b5mo_path<<std::endl;
        std::cerr<<"b5mp_path: "<<b5mp_path<<std::endl;
        std::cerr<<"b5ma_path: "<<b5ma_path<<std::endl;
        std::cerr<<"b5mq_path: "<<b5mq_path<<std::endl;
        std::cerr<<"b5mc_path: "<<b5mc_path<<std::endl;
        std::cerr<<"scd_path: "<<scd_path<<std::endl;
        std::cerr<<"comment_scd_path: "<<comment_scd_path<<std::endl;
        std::cerr<<"knowledge: "<<knowledge<<std::endl;
        std::cerr<<"addr_knowledge: "<<addr_knowledge<<std::endl;
        std::cerr<<"tag_knowledge: "<<tag_knowledge<<std::endl;
        std::cerr<<"maxent_title_knowledge: "<<maxent_title_knowledge<<std::endl;
        std::cerr<<"cma_path: "<<cma_path<<std::endl;
        std::cerr<<"rank_scd_path: "<<rank_scd_path<<std::endl;
        std::cerr<<"gen_b5ma: "<<gen_b5ma<<std::endl;
        std::cerr<<"gen_b5mq: "<<gen_b5mq<<std::endl;
        std::cerr<<"rtype: "<<rtype<<std::endl;
        std::cerr<<"gen_tags: "<<gen_tags<<std::endl;
        std::cerr<<"gen_oextra: "<<gen_oextra<<std::endl;
        std::cerr<<"mobile_source: "<<mobile_source<<std::endl;
        std::cerr<<"human_match: "<<human_match<<std::endl;
        std::cerr<<"buffer_size: "<<buffer_size<<std::endl;
        std::cerr<<"sorter_bin: "<<sorter_bin<<std::endl;
        std::cerr<<"classifier_ip: "<<classifier_ip<<std::endl;
        std::cerr<<"classifier_port: "<<classifier_port<<std::endl;
        std::cerr<<"comment_ip: "<<comment_ip<<std::endl;
        std::cerr<<"comment_port: "<<comment_port<<std::endl;
        std::cerr<<"brand_ip: "<<brand_ip<<std::endl;
        std::cerr<<"brand_port: "<<brand_port<<std::endl;
    }

    std::string path;
    std::string ts;
    std::string b5mo_path;
    std::string b5mp_path;
    std::string b5mc_path;
    std::string b5ma_path;
    std::string b5mq_path;
    std::string scd_path;
    std::string comment_scd_path;
    std::string rank_scd_path;
    int mode;
    int cmode;
    int thread_num;
    bool gen_b5ma;
    bool gen_b5mq;
    bool rtype;
    bool gen_tags;
    bool gen_oextra;
    std::string mobile_source;
    std::string human_match;
    std::string knowledge;
    std::string addr_knowledge;
    std::string tag_knowledge;
    std::string maxent_title_knowledge;
    std::string cma_path;
    std::string buffer_size;
    std::string sorter_bin;
    std::string classifier_ip;
    int classifier_port;
    std::string comment_ip;
    int comment_port;
    std::string brand_ip;
    int brand_port;

    static bool GetLatestScdPath(const std::string& dir, std::string& scd_path)
    {
        namespace bfs = boost::filesystem;
        if(!bfs::is_directory(dir)) return false;
        bfs::path p(dir);
        bfs::directory_iterator end;
        std::vector<std::string> candidates;
        for(bfs::directory_iterator it(p);it!=end;it++)
        {
            if(bfs::is_directory(it->path()))
            {
                bfs::path done = it->path()/"done";
                if(!bfs::exists(done)) continue;
                std::string dir_name = it->path().filename().string();
                bool valid = true;
                if(dir_name.length()!=14) valid = false;
                else
                {
                    for(std::size_t i=0;i<dir_name.length();i++)
                    {
                        char c = dir_name[i];
                        if(c<'0' || c>'9')
                        {
                            valid = false;
                            break;
                        }
                    }
                }
                if(!valid) continue;
                candidates.push_back(dir_name);
            }
        }
        if(candidates.empty()) return false;
        std::sort(candidates.begin(), candidates.end());
        bfs::path rpath = p/candidates.back();
        scd_path = rpath.string();
        return true;
    }
private:
    template <typename T>
    bool SetValue_(const YAML::Node& node, T& value)
    {
        if(node.Type()==YAML::NodeType::Undefined||node.Type()==YAML::NodeType::Null) return false;
        value = node.as<T>();
        return true;
    }
    void ShowNode_(const YAML::Node& node)
    {
        for(YAML::const_iterator it = node.begin();it!=node.end();++it)
        {
            std::cout<<it->first.as<std::string>()<<" is "<<it->second.Type()<<std::endl;
        }
    }
    void SetDefaultValue_(const std::string& m)
    {
        path = m;
        b5mo_path = m+"/b5mo";
        b5mp_path = m+"/b5mp";
        b5mc_path = m+"/b5mc";
        b5ma_path = m+"/b5ma";
        b5mq_path = m+"/b5mq";
        ts = boost::filesystem::path(m).filename().string();
        if(ts==".")
        {
            ts = boost::filesystem::path(m).parent_path().filename().string();
        }
        gen_b5ma = false;
        gen_b5mq = false;
        rtype = false;
        gen_tags = false;
        gen_oextra = false;
    }

};
NS_IDMLIB_B5M_END

#endif

