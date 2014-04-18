#ifndef IDMLIB_UTIL_SVMMODEL_H_
#define IDMLIB_UTIL_SVMMODEL_H_


#include <idmlib/idm_types.h>
#include "svm.h"
#include <am/sequence_file/ssfr.h>
#include <boost/unordered_map.hpp>
NS_IDMLIB_UTIL_BEGIN



class SvmModel
{
#define Malloc(type,n) (type *)malloc((n)*sizeof(type))
public:
    struct Feature
    {
        std::string text;
        double weight;
        int id;
    };
    struct Instance
    {
        std::vector<Feature> features;
        double label;
    };
    struct IdManager
    {
        typedef boost::unordered_map<std::string, int> Map;
        Map map;
        int max;
        IdManager():max(0)
        {
        }
        int GetIdByText(const std::string& text)
        {
            Map::const_iterator it = map.find(text);
            if(it!=map.end())
            {
                return it->second;
            }
            else
            {
                int id = ++max;
                map.insert(std::make_pair(text, id));
                return id;
            }
        }
        bool GetIdByText(const std::string& text, int& id)
        {
            Map::const_iterator it = map.find(text);
            if(it!=map.end())
            {
                id = it->second;
                return true;
            }
            else
            {
                return false;
            }
        }
        friend class boost::serialization::access;
        template<class Archive>
        void save(Archive & ar, const unsigned int version) const
        {
            ar & max;
            std::map<std::string, int> stdmap(map.begin(), map.end());
            ar & stdmap;
        }
        template<class Archive>
        void load(Archive & ar, const unsigned int version)
        {
            ar & max;
            std::map<std::string, int> stdmap;
            ar & stdmap;
            map.insert(stdmap.begin(), stdmap.end());
        }
        BOOST_SERIALIZATION_SPLIT_MEMBER();
    };
    SvmModel(IdManager* id_manager=NULL):model_(NULL),id_manager_(id_manager)
    {
    }
    ~SvmModel()
    {
        if(model_!=NULL)
        {
            svm_free_and_destroy_model(&model_);
        }
    }
    void SetIdManager(IdManager* idm)
    {
        id_manager_ = idm;
    }
    bool Open(const std::string& path)
    {
        std::string fpath = path+"/name";
        izenelib::am::ssf::Util<>::Load(fpath, name_);
        std::string model_file = path+"/model";
        if( (model_=svm_load_model(model_file.c_str()))==0 )
        {
            LOG(ERROR)<<"load model failed"<<std::endl;
            return false;
        }
        return true;
    }

    bool TrainStart(const std::string& path, const std::string& name="")
    {
        path_ = path;
        name_ = name;
        max_id_ = 0;
        svm_parameter& param = param_;
        param.svm_type = C_SVC;
        param.kernel_type = RBF;
        param.degree = 3;
        param.gamma = 0.0;	// 1/num_features
        param.coef0 = 0;
        param.nu = 0.5;
        param.cache_size = 100;
        param.C = 32;
        param.eps = 1e-3;
        param.p = 0.1;
        param.shrinking = 1;
        param.probability = 0;
        param.nr_weight = 0;
        param.weight_label = NULL;
        param.weight = NULL;
        return true;
    }
    void Add(const Instance& ins)
    {
        instance_list_.push_back(ins);
        Instance& inst = instance_list_.back();
        for(uint32_t i=0;i<inst.features.size();i++)
        {
            Feature& f = inst.features[i];
            f.id = id_manager_->GetIdByText(f.text);
        }
    }
    const std::string& Name() const
    {
        return name_;
    }
    struct svm_parameter& SvmParam()
    {
        return param_;
    }
    bool TrainEnd()
    {
        struct svm_node* x_space;
        struct svm_problem problem;
        problem.l = instance_list_.size();
        problem.y = Malloc(double, problem.l);
        problem.x = Malloc(struct svm_node*, problem.l);
        uint32_t elements = 0;
        for(uint32_t i=0;i<instance_list_.size();i++)
        {
            elements += instance_list_[i].features.size()+1;
        }
        x_space = Malloc(struct svm_node, elements);
        int n = 0;
        int max_index = 0;
        for(int i=0;i<problem.l;i++)
        {
            const Instance& ins = instance_list_[i];
            problem.x[i] = &x_space[n];
            problem.y[i] = ins.label;
            for(uint32_t j=0;j<ins.features.size();j++)
            {
                const Feature& f = ins.features[j];
                x_space[n].index = f.id;
                x_space[n].value = f.weight;
                if(x_space[n].index>max_index)
                {
                    max_index = x_space[n].index;
                }
                ++n;
            }
            x_space[n++].index = -1;
        }
        instance_list_.clear();
        if(param_.gamma == 0.0 && max_index>0)
        {
            param_.gamma = 1.0/max_index;
        }
        boost::filesystem::create_directories(path_);
        std::string model_file = path_+"/model";
        model_ = svm_train(&problem, &param_);
        if(svm_save_model(model_file.c_str(), model_))
        {
            LOG(ERROR)<<"can not save svm model"<<std::endl;
            return false;
        }
        svm_free_and_destroy_model(&model_);
        model_ = NULL;
        svm_destroy_param(&param_);
        free(problem.y);
        free(problem.x);
        free(x_space);
        std::string path = path_+"/name";
        izenelib::am::ssf::Util<>::Save(path, name_);
        return true;
    }

    double Test(const Instance& ins, std::vector<double>& probs)
    {
        std::vector<std::pair<int, double> > values;
        for(uint32_t i=0;i<ins.features.size();i++)
        {
            const Feature& f = ins.features[i];
            int id = 0;
            if(id_manager_->GetIdByText(f.text, id))
            {
                values.push_back(std::make_pair(id, f.weight));
            }
        }
        if(values.empty())
        {
            return 0.0;
        }
        struct svm_node *x = (struct svm_node *) malloc((values.size()+1)*sizeof(struct svm_node));
        for(uint32_t i=0;i<values.size();i++)
        {
            x[i].index = values[i].first;
            x[i].value = values[i].second;
        }
        x[values.size()].index = -1;
        int nr_class = svm_get_nr_class(model_);
        double* v = new double[nr_class];
        double label = svm_predict_probability(model_,x,v);
        for(int i=0;i<nr_class;i++)
        {
            probs.push_back(v[i]);
        }
        free(x);
        delete[] v;
        return label;
    }


private:

    std::string path_;
    std::string name_;
    struct svm_model* model_; 
    struct svm_parameter param_;
    std::vector<Instance> instance_list_;
    int max_id_;
    IdManager* id_manager_;


};

NS_IDMLIB_UTIL_END

#endif
