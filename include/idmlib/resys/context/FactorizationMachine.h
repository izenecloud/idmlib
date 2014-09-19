#ifndef IDMLIB_RESYS_FACTORIZATION_MACHINE_H
#define IDMLIB_RESYS_FACTORIZATION_MACHINE_H

#include <idmlib/idm_types.h>
#include <idmlib/resys/context/FactorContainer.h>

#include <am/graphchi/graphchi_basic_includes.hpp>
#include <util/izene_serialization.h>

#include <boost/type_traits.hpp>
#include <boost/function.hpp>

#include <map>

namespace idmlib{ namespace recommender{ 

using namespace graphchi;

#pragma pack(push,1)
template<int NLATENT = 20>
struct vertex_data
{
    float pvec[NLATENT];
    float bias;
    vertex_data() 
    {
        for(int k=0; k < NLATENT; k++) 
           pvec[k] =  drand48(); 
        bias = 0;
    }
	
    template<class DataIO> friend
    void DataIO_loadData(DataIO& dio, vertex_data& x)
    {
        dio.ensureRead(static_cast<void*>(x.pvec), sizeof(x.pvec));
        dio & x.bias;
    }
  

    template<class DataIO> friend
    void DataIO_saveData(DataIO& dio, const vertex_data& x)
    {
        dio.ensureWrite(static_cast<void*>(x.pvec), sizeof(x.pvec));
        dio & x.bias;
    }
};
#pragma pack(pop)


template<int NUMCONTEXT>
struct edge_data
{
    float weight;
    unsigned contexts[NUMCONTEXT];
    unsigned get_context(int contextId)const { return contexts[contextId]; }
    int num_contexts() const { return NUMCONTEXT; }
};

template<>
struct edge_data<0>
{
    float weight;
    unsigned get_context(int contextId)const { return 0; }
    int num_contexts() const { return 0; }
};


template<
    int NLATENT = 20,
    typename VertexDataType = vertex_data<NLATENT>,
    typename EdgeDataType = edge_data<0>, 
    bool HASIMPLICIT = false,
    typename FactorContainerType = FactorContainerInMem<VertexDataType >
>
class FactorizationMachine 
    : public GraphChiProgram<VertexDataType, EdgeDataType> 
{
    enum { LOSS_FUNCTION_SIGMOID, LOSS_FUNCTION_LN_SIGMOID};

    std::string factor_store_path_;

    unsigned int num_context_;
    ///Attention: In order to support dynamic building, each factor will 
    ///occupy separate factor container to be able to increase dynamically
    ///We also need to make assumption as follows:
    ///factors[0]   users
    ///factors[1]   items
    ///context information that will be reflected in feedback data(also in edge_data)
    ///will start from factors[2]
    ///context information that will not be reflected in feedback data(not appear in
    ///edge_data) will follow accordingly
    FactorContainerType ** factors_;

    static const float minval_ ;

    static const float maxval_ ;
public:
    float reg0_;

    float w0_;

    float learn_rate_;

    float regw_;

    float regv_;
	
    explicit FactorizationMachine(
        const std::string& factor_store_path,
        unsigned int num_context,
        float learn_rate,
        float regw,
        float regv
    )
        :factor_store_path_(factor_store_path)
        ,num_context_(num_context + 2)
        ,reg0_(0.1)
        ,w0_(0)
        ,learn_rate_(learn_rate)
        ,regw_(regw)
        ,regv_(regv)
    {
        factors_ = new FactorContainerType * [num_context_];
        for(unsigned i = 0; i < num_context_; ++i)
        {
            factors_[i] = new FactorContainerType(factor_store_path_);
        }
    }

    ~FactorizationMachine()
    {
        for(unsigned i = 0; i < num_context_; ++i)
            delete factors_[i];
        delete [] factors_;
    }

    void init_factors(const std::vector<unsigned>& sizes )
    {
        if(sizes.size() != num_context_)
            throw std::runtime_error("Each dimensional of context should contain a legial size");
        for(unsigned i = 0; i < sizes.size(); ++i) 
        {
            if(factors_[i])
                factors_[i]->Init(sizes[i]);
            else
                factors_[i] = new FactorContainerType(factor_store_path_);				
        }
    }

    void add_user(unsigned user_id)  { add_context_(0, user_id); }

    void add_item(unsigned item_id) { add_context_(1, item_id); }

    void add_context(unsigned context, unsigned context_id) { add_context_(context + 2, context_id); }

    void before_iteration(int iteration, graphchi_context &info) {}
    
    void after_iteration(int iteration, graphchi_context &ginfo) {}
    
    void before_exec_interval(vid_t window_st, vid_t window_en, graphchi_context &ginfo) {}

    void after_exec_interval(vid_t window_st, vid_t window_en, graphchi_context &gcontext) {}

    void update(
        graphchi_vertex<VertexDataType, EdgeDataType> &vertex, 
        graphchi_context &ginfo) 
    {
        update_(vertex, ginfo, boost::integral_constant<bool, HASIMPLICIT>());
    }

    float predict(const std::vector<uint32_t>& features)
    {
        std::vector<uint32_t> sum;
        return predict_(features, sum);
    }

protected:
    DISALLOW_COPY_AND_ASSIGN(FactorizationMachine);	

    float predict_(
        const std::vector<uint32_t>& features,
        std::vector<uint32_t>& sum
        )
    {
        float prediction = w0_;
        std::vector<uint32_t> sum_sqr(NLATENT);
        sum.resize(NLATENT);

        for (unsigned i = 0; i < features.size(); i++) 
        {
            prediction += factors_[i]->operator[](features[i]).bias;
        }
		
        for (unsigned f = 0; f < NLATENT; ++f) 
        {
            sum[f] = 0;
            sum_sqr[f] = 0;
            for (unsigned i = 0; i < features.size(); i++) 
            {
                const VertexDataType& vertex = factors_[i]->operator[](features[i]);
                float d = vertex.pvec[f];
                sum[f] += d;
                sum_sqr[f] += d*d;
            }
            prediction += 0.5 * (sum[f]*sum[f] - sum_sqr[f]);
        }
        return prediction;
    }

    /// General SGD iteration
    void update_(
        graphchi_vertex<VertexDataType, EdgeDataType> &vertex, 
        graphchi_context &ginfo,
        const boost::false_type&
        ) 
    {
        if ( vertex.num_outedges() > 0)
        {
            VertexDataType & user = factors_[0]->operator[](vertex.id()); 
            std::vector<uint32_t> features;
            std::vector<uint32_t> sum;
            for(int e = 0; e < vertex.num_edges(); ++e) 
            {
                features.resize(2);
                features[0] = vertex.id();
                features[1] = vertex.edge(e)->vertex_id();
                float pui;
                VertexDataType & item = factors_[1]->operator[](vertex.edge(e)->vertex_id()); 
                EdgeDataType edge = vertex.edge(e)->get_data();

                if(0 != edge.num_contexts())
                {
                    for(int ci = 0; ci < edge.num_contexts(); ++ci)
                    {
                        features.push_back(edge.get_context(ci));
                    }
                }
                pui = predict_(features, sum);
                float eui = pui - edge.weight;
                w0_ -= learn_rate_ * (eui + reg0_ * w0_);
                user.bias -= learn_rate_ * (eui + regw_ * user.bias);
                item.bias -= learn_rate_ * (eui + regw_ * item.bias);
                if(0 != edge.num_contexts())
                {
                    for(int ci = 0; ci < edge.num_contexts(); ++ci)
                    {
                        VertexDataType& context = factors_[ci + 2]->operator[](edge.get_context(ci));
                        context.bias -= learn_rate_ * (eui + regw_ * context.bias);
                    }
                }

                for(int f = 0; f < NLATENT; f++)
                {
                    // user
                    float grad = sum[f] - user.pvec[f];
                    user.pvec[f] -= learn_rate_ * (eui * grad + regv_ * user.pvec[f]);
                    // item
                    grad = sum[f] - item.pvec[f];
                    item.pvec[f] -= learn_rate_ * (eui * grad + regv_ * item.pvec[f]);
                    // context
                    if(0 != edge.num_contexts())
                    {
                        for(int ci = 0; ci < edge.num_contexts(); ++ci)
                        {
                            VertexDataType& context = factors_[ci + 2]->operator[](edge.get_context(ci));
                            grad = sum[f] - context.pvec[f];
                            context.pvec[f] -= learn_rate_ * (eui * grad + regv_ * context.pvec[f]);
                        }
                    }
                }
            }
        }
    }

    /// SGD iteration with implicit feedback
    /// For implicit feedback, all weight are 1(positive) or -1(negative)
    void update_(
        graphchi_vertex<VertexDataType, EdgeDataType> &vertex, 
        graphchi_context &ginfo,
        const boost::true_type&
        ) 
    {
        if ( vertex.num_outedges() > 0)
        {
            VertexDataType & user = factors_[0]->operator[](vertex.id()); 
            std::vector<uint32_t> features_pos, features_neg;
            std::vector<uint32_t> sum_pos, sum_neg;
            std::map<uint32_t, bool> grad_visited;

            for(int e = 0; e < vertex.num_edges(); ++e) 
            {
                features_pos.resize(2);
                features_pos[0] = vertex.id();
                features_pos[1] = vertex.edge(e)->vertex_id();
                float pui_pos, pui_neg;
                EdgeDataType edge = vertex.edge(e)->get_data();

                if(0 != edge.num_contexts())
                {
                    for(int ci = 0; ci < edge.num_contexts(); ++ci)
                    {
                        features_pos.push_back(edge.get_context(ci));
                    }
                }
                pui_pos = predict_(features_pos, sum_pos);

                features_neg.resize(2);
                features_neg[0] = vertex.id();
                features_neg[1] = get_implicit_random_(factors_[1]->Size());

                if(0 != edge.num_contexts())
                {
                    for(int ci = 0; ci < edge.num_contexts(); ++ci)
                    {
                        features_neg.push_back(get_implicit_random_(factors_[ci]->Size()));
                    }
                }
                pui_neg = predict_(features_neg, sum_neg);
                float multiplier = -(partial_loss_(LOSS_FUNCTION_LN_SIGMOID, pui_pos - pui_neg));
				
                w0_ -= reg0_ * w0_;
                /// user
                user.bias -= learn_rate_ * (multiplier + regw_ * user.bias);
                /// other context
                for (unsigned i = 1; i < features_pos.size(); ++i) 
                {
                    grad_visited[features_pos[i]] = false;
                }
                for (unsigned i = 1; i < features_neg.size(); ++i) 
                {
                    grad_visited[features_neg[i]] = false;
                }
                for (unsigned i = 1; i < features_pos.size(); ++i) 
                {
                    unsigned& attr_id = features_pos[i];
                    if (! grad_visited[attr_id]) 
                    {
                        VertexDataType& context = factors_[i]->operator[](attr_id);
                        context.bias -= learn_rate_ * (multiplier + regw_ * context.bias);
                        grad_visited[attr_id] = true;
                    }
                }
                for (unsigned i = 1; i < features_neg.size(); ++i) 
                {
                    unsigned& attr_id = features_neg[i];
                    if (! grad_visited[attr_id]) 
                    {
                        VertexDataType& context = factors_[i]->operator[](attr_id);
                        context.bias -= learn_rate_ * (-multiplier + regw_ * context.bias);
                        grad_visited[attr_id] = true;
                    }
                }			

                for(int f = 0; f < NLATENT; f++)
                {
                    /// user
                    user.pvec[f] -= learn_rate_ * (multiplier * (sum_pos[f] - user.pvec[f]) + regv_ * user.pvec[f]);
                    /// other context
                    for (unsigned i = 1; i < features_pos.size(); ++i) 
                    {
                        grad_visited[features_pos[i]] = false;
                    }
                    for (unsigned i = 1; i < features_neg.size(); ++i) 
                    {
                        grad_visited[features_neg[i]] = false;
                    }
                    for (unsigned i = 1; i < features_pos.size(); ++i) 
                    {
                        unsigned& attr_id = features_pos[i];
                        if (! grad_visited[attr_id]) 
                        {
                            VertexDataType& context = factors_[i]->operator[](attr_id);
                            float grad = sum_pos[f] - context.pvec[f];
                            context.pvec[f] -= learn_rate_ * (multiplier * grad + regv_ * context.pvec[f]);
                            grad_visited[attr_id] = true;
                        }
                    }
                    for (unsigned i = 1; i < features_neg.size(); ++i) 
                    {
                        unsigned& attr_id = features_neg[i];
                        if (! grad_visited[attr_id]) 
                        {
                            VertexDataType& context = factors_[i]->operator[](attr_id);
                            float grad = sum_neg[f] - context.pvec[f];
                            context.pvec[f] -= learn_rate_ * (multiplier * grad + regv_ * context.pvec[f]);
                            grad_visited[attr_id] = true;
                        }
                    }	
                }
            }
        }
    }

    unsigned get_implicit_random_(unsigned upper_limit)
    {
        return rand() % upper_limit;
    }

    float partial_loss_(int loss_function, float x) 
    {
        switch(loss_function)
        {
        case LOSS_FUNCTION_SIGMOID:
        {
            float sigmoid_tp_tn = (float) 1 / (1+exp(-x));
            return sigmoid_tp_tn * (1-sigmoid_tp_tn);
        }
        case LOSS_FUNCTION_LN_SIGMOID:
        {
            float exp_x = exp(-x);
            return exp_x / (1 + exp_x);
        } 
        default:
            return 0;
        }
    }

    void add_context_(unsigned context, unsigned context_id)
    {
        if(factors_[context])
        {
            if(context_id >= factors_[context].size())
            {
                factors_[context]->Init(context_id);
            }
        }
    }

    void set_latent_factor_(
         graphchi_vertex<VertexDataType, EdgeDataType> &vertex, 
         VertexDataType &fact) 
    {
        vertex.set_data(fact); // Note, also stored on disk. This is non-optimal...
        for(int i=0; i < vertex.num_edges(); i++) {
//            als_factor_and_weight factwght = vertex.edge(i)->get_data();
//            factwght.factor = fact;
//            vertex.edge(i)->set_data(factwght);   // Note that neighbors override the values they have written to edges.
                                                  // This is ok, because vertices are always executed in same order.
        }
        
        factors_[0]->operator[](vertex.id()) = fact;
    }

};


template<
    int NLATENT,
    typename VertexDataType ,
    typename EdgeDataType , 
    bool HASIMPLICIT ,
    typename FactorContainerType 
>
const float FactorizationMachine<NLATENT, VertexDataType, EdgeDataType, HASIMPLICIT, FactorContainerType>::minval_ = -1e100;

 
template<
    int NLATENT,
    typename VertexDataType ,
    typename EdgeDataType , 
    bool HASIMPLICIT ,
    typename FactorContainerType 
>
const float FactorizationMachine<NLATENT, VertexDataType, EdgeDataType, HASIMPLICIT, FactorContainerType>::maxval_ = 1e100;


}}

namespace izenelib{namespace util{
template <int NLATENT>
struct IsFebirdSerial< idmlib::recommender::vertex_data<NLATENT> >
{
    enum { yes = 1, no = !yes };
};
}}

#endif

