#ifndef SGD_H_
#define SGD_H_

#include <utility>
#include <string>
#include <vector>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <fstream>
#include "fm_model.h"
#include "sparse_data.h"

namespace idmlib {

class SGD {
public:
    fm_model *fm;
    sparse_data *train;
    sparse_data *test;
    double learn_rate;
    double reg_w0, reg_w, reg_v;
    int num_iter;

    SGD(fm_model *fm, sparse_data *train, sparse_data *test, double learn_rate, double reg_w0, double reg_w, double reg_v, int num_iter)
        : fm(fm),
          train(train),
          test(test),
          learn_rate(learn_rate),
          reg_w0(reg_w0),
          reg_w(reg_w),
          reg_v(reg_v),
          num_iter(num_iter)
    {
    }

    void learn()
    {
        std::vector<double> sum(fm->v.size());
        std::vector<double> sum_sqr(fm->v.size());
        for (int i = 0; i != num_iter; ++i) {
            for (unsigned long j = 0; j != (unsigned long)train->X.size(); ++j) {
                double p = fm->predict(train->X[j], sum, sum_sqr);
                p = std::min(train->maxY, p);
                p = std::max(train->minY, p);

                double mult = p - train->Y[j];
                update(train->X[j], mult, sum);
            }
            evaluate(i);
        }
    }

    void update(std::vector<std::pair<unsigned long, double> > & row, double multiplier, std::vector<double> & sum)
    {
        double & w0 = fm->w0;
        w0 -= learn_rate * (multiplier + reg_w0 * w0);

        for (int i = 0; i != (int)row.size(); ++i) {
            double & w = fm->w[row[i].first];
            w -= learn_rate * (multiplier * row[i].second + reg_w * w);
        }

        for (int i = 0; i != (int)fm->v.size(); ++i) {
            for (int j = 0; j != (int)row.size(); ++j) {
                double & v = fm->v[i][row[j].first];
                double grad = sum[i] * row[j].second - v * row[j].second * row[j].second;
                v -= learn_rate * (multiplier * grad + reg_v * v);
            }
        }
    }

    double evaluate(sparse_data *data)
    {
        std::vector<double> sum(fm->v.size());
        std::vector<double> sum_sqr(fm->v.size());

        double rmse_sum_sqr = 0.0;
        for (unsigned long i = 0; i != (unsigned long)data->X.size(); ++i) {
            double p = fm->predict(data->X[i], sum, sum_sqr);
            p = std::min(train->maxY, p);
            p = std::max(train->minY, p);
            double err = p - data->Y[i];
            rmse_sum_sqr += err * err;
        }
        return std::sqrt(rmse_sum_sqr / data->X.size());
    }

    void evaluate(int i)
    {
        std::cout << "#Iter=" << std::setw(3) << i << "\tTrain=" << evaluate(train) << "\tTest=" << evaluate(test) << std::endl;
    }

    void dump(const std::string & filename)
    {
        std::vector<double> sum(fm->v.size());
        std::vector<double> sum_sqr(fm->v.size());

        std::ofstream out(filename.c_str());
        for (unsigned long i = 0; i != (unsigned long)test->X.size(); ++i) {
            double p = fm->predict(test->X[i], sum, sum_sqr);
            p = std::min(train->maxY, p);
            p = std::max(train->minY, p);
            out << p << std::endl;
        }
    }
};

}

#endif
