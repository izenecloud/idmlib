#ifndef ALS_H_
#define ALS_H_

#include <vector>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <utility>
#include <string>
#include <fstream>
#include "fm_model.h"
#include "sparse_data.h"

namespace idmlib {

class ALS {
public:
    fm_model *fm;
    sparse_data *train;
    sparse_data *test;
    double reg_w0, reg_w, reg_v;
    int num_iter;

    std::vector<double> e;
    std::vector<double> q;

    ALS(fm_model *fm, sparse_data *train, sparse_data *test, double reg_w0, double reg_w, double reg_v, int num_iter)
        : fm(fm),
          train(train),
          test(test),
          reg_w0(reg_w0),
          reg_w(reg_w),
          reg_v(reg_v),
          num_iter(num_iter),
          e(train->X.size(), 0.0),
          q(train->X.size(), 0.0)
    {
    }

    void learn()
    {
        std::vector<double> sum(fm->v.size());
        std::vector<double> sum_sqr(fm->v.size());

        // initialize e...
        for (unsigned long i = 0; i != (unsigned long)train->X.size(); ++i) {
            e[i] = fm->predict(train->X[i], sum, sum_sqr) - train->Y[i];
        }

        for (int i = 0; i != num_iter; ++i) {
            draw_w0(fm->w0);

            for (unsigned long j = 0; j != (unsigned long)train->X_t.size(); ++j) {
                draw_w(fm->w[j], train->X_t[j]);
            }

            for (int k = 0; k != (int)fm->v.size(); ++k) {
                init_q(k);
                for (unsigned long p = 0; p != (unsigned long)train->X_t.size(); ++p) {
                    draw_v(fm->v[k][p], train->X_t[p]);
                }
            }

            evaluate(i);
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

    void draw_w0(double &w0)
    {
        double w0_new = 0.0;
        for (unsigned long i = 0; i != (unsigned long)train->X.size(); ++i) {
            w0_new += e[i] - w0;
        }
        w0_new *= -1.0 / (train->X.size() + reg_w0);

        for (unsigned long i = 0; i != (unsigned long)train->X.size(); ++i) {
            e[i] += w0_new - w0;
        }

        w0 = w0_new;
    }

    void draw_w(double &w, std::vector<std::pair<unsigned long, double> > &feature_col)
    {
        double mean = 0.0;
        double var = 0.0;
        for (unsigned long i = 0; i != (unsigned long)feature_col.size(); ++i) {
            unsigned long &row = feature_col[i].first;
            double &value = feature_col[i].second;
            mean += (e[row] - w * value) * value;
            var += value * value;
        }
        double w_new = -mean / (var + reg_w);

        for (unsigned long i = 0; i != (unsigned long)feature_col.size(); ++i) {
            unsigned long &row = feature_col[i].first;
            double &value = feature_col[i].second;
            e[row] += (w_new - w) * value;
        }

        w = w_new;
    }

    void init_q(int k)
    {
        for (unsigned long i = 0; i != (unsigned long)train->X.size(); ++i) {
            q[i] = 0.0;
            for (int j = 0; j != (int)train->X[i].size(); ++j) {
                q[i] += fm->v[k][train->X[i][j].first] * train->X[i][j].second;
            }
        }
    }

    void draw_v(double &v, std::vector<std::pair<unsigned long, double> > &feature_col)
    {
        double mean = 0.0;
        double var = 0.0;
        for (unsigned long i = 0; i != (unsigned long)feature_col.size(); ++i) {
            unsigned long &row = feature_col[i].first;
            double &value = feature_col[i].second;
            double h = value * (q[row] - v * value);
            mean += (e[row] - v * h) * h;
            var += h * h;
        }
        double v_new = -mean / (var + reg_v);

        for (unsigned long i = 0; i != (unsigned long)feature_col.size(); ++i) {
            unsigned long &row = feature_col[i].first;
            double &value = feature_col[i].second;
            double h = value * (q[row] - v * value);
            e[row] += (v_new - v) * h;
            q[row] += (v_new - v) * value;
        }

        v = v_new;
    }
};

}

#endif
