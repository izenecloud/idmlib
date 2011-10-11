#ifndef LABELED_LDA_H__
#define LABELED_LDA_H__

#include <idmlib/topic-models/common/samplib.h>
#include <idmlib/topic-models/common/stats.h>

#include <string>

namespace idmlib{ namespace topicmodels{

class LabeledLDA
{
public:
    std::string training_data;
    std::string label_data;

    int D;
    int K;
    int** M;
    int* N;
    int V;
    double alpha;
    double eta;
    double** post_beta;
    double** post_theta;
    int*** w;
    int*** z;
    int*** best_z;
    int** labels;
    int nIter;
    int dumpInterval;

    LabeledLDA(const std::string& trainingfile, const std::string& labelfile, int iter);

    ~LabeledLDA();

    void estimate();

private:
    void load_model();

    void load_labels();

    void initialize_model();

    void initialize_alpha();

    void initialize_eta();

    void initialize_z();

    void initialize_post_beta();

    void initialize_post_theta();

    void resample_post_beta();

    void resample_post_theta();

    void resample_z();

    double compute_log_posterior();

    void dump_z();

    void clean_model();
};

}}
#endif

