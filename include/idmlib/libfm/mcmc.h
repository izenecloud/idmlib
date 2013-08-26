#ifndef MCMC_H_
#define MCMC_H_

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

class MCMC {
public:
    fm_model *fm;
    sparse_data *train;
    sparse_data *test;
    int num_iter;

    std::vector<double> e;
    std::vector<double> q;

    double alpha;
    double w_mu;
    double w_lambda;
    std::vector<double> v_mu;
    std::vector<double> v_lambda;

    std::vector<double> pred;

    MCMC(fm_model *fm, sparse_data *train, sparse_data *test, int num_iter)
        : fm(fm),
          train(train),
          test(test),
          num_iter(num_iter),
          e(train->X.size(), 0.0),
          q(train->X.size(), 0.0),
          v_mu(fm->v.size(), 0.0),
          v_lambda(fm->v.size(), 0.0),
          pred(test->X.size(), 0.0)
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
            draw_alpha();
            draw_w0(fm->w0);
            draw_w_lambda();
            draw_w_mu();
            for (unsigned long j = 0; j != (unsigned long)train->X_t.size(); ++j) {
                draw_w(fm->w[j], train->X_t[j]);
            }

            draw_v_lambda();
            draw_v_mu();
            for (int k = 0; k != (int)fm->v.size(); ++k) {
                init_q(k);
                for (unsigned long p = 0; p != (unsigned long)train->X_t.size(); ++p) {
                    draw_v(fm->v[k][p], v_mu[k], v_lambda[k], train->X_t[p]);
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

    double evaluate(sparse_data *data, std::vector<double> &pred)
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

            pred[i] += p;
        }
        return std::sqrt(rmse_sum_sqr / data->X.size());
    }

    void evaluate(int i)
    {
        std::cout << "#Iter=" << std::setw(3) << i << "\tTrain=" << evaluate(train) << "\tTest=" << evaluate(test, pred) << std::endl;
    }

    void dump(const std::string & filename)
    {
        std::ofstream out(filename.c_str());
        for (unsigned long i = 0; i != (unsigned long)test->X.size(); ++i) {
            out << pred[i] / num_iter << std::endl;
        }
    }

    void draw_alpha()
    {
        double alpha_n = 1.0 + train->X.size();
        double beta_n = 1.0;
        for (unsigned long i = 0; i != (unsigned long)train->X.size(); ++i) {
            beta_n += e[i] * e[i];
        }
        alpha = ran_gamma(alpha_n / 2.0, beta_n / 2.0);
    }

    void draw_w0(double &w0)
    {
        double var = 1.0 / (alpha * train->X.size());
        double mean = 0.0;
        for (unsigned long i = 0; i != (unsigned long)train->X.size(); ++i) {
            mean += e[i] - w0;
        }
        mean = -var * (alpha * mean);
        double w0_new = ran_gaussian(mean, std::sqrt(var));

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
            mean += value * (e[row] - w * value);
            var += value * value;
        }
        var = 1.0 / (w_lambda + alpha * var);
        mean = -var * (alpha * mean - w_mu * w_lambda);
        double w_new = ran_gaussian(mean, std::sqrt(var));

        for (unsigned long i = 0; i != (unsigned long)feature_col.size(); ++i) {
            unsigned long &row = feature_col[i].first;
            double &value = feature_col[i].second;
            e[row] += (w_new - w) * value;
        }

        w = w_new;
    }

    void draw_v(double &v, double &v_mu, double &v_lambda, std::vector<std::pair<unsigned long, double> > &feature_col)
    {
        double mean = 0.0;
        double var = 0.0;
        for (unsigned long i = 0; i != (unsigned long)feature_col.size(); ++i) {
            unsigned long &row = feature_col[i].first;
            double &value = feature_col[i].second;
            double h = value * (q[row] - v * value);
            mean += h * e[row];
            var += h * h;
        }
        mean -= v * var;
        var = 1.0 / (v_lambda + alpha * var);
        mean = -var * (alpha * mean - v_mu * v_lambda);
        double v_new = ran_gaussian(mean, std::sqrt(var));

        for (unsigned long i = 0; i != (unsigned long)feature_col.size(); ++i) {
            unsigned long &row = feature_col[i].first;
            double &value = feature_col[i].second;
            double h = value * (q[row] - v * value);
            e[row] += (v_new - v) * h;
            q[row] += (v_new - v) * value;
        }

        v = v_new;
    }

    void draw_w_mu()
    {
        double mean = 0.0;
        for (unsigned long i = 0; i != (unsigned long)fm->w.size(); ++i) {
            mean += fm->w[i];
        }
        mean /= 1.0 + fm->w.size();
        double var = 1.0 / ((1.0 + fm->w.size()) * w_lambda);
        w_mu = ran_gaussian(mean, std::sqrt(var));
    }

    void draw_w_lambda()
    {
        double beta = w_mu * w_mu + 1.0;
        for (unsigned long i = 0; i != (unsigned long)fm->w.size(); ++i) {
            beta += (fm->w[i] - w_mu) * (fm->w[i] - w_mu);
        }
        double alp = 2.0 + fm->w.size();
        w_lambda = ran_gamma(alp / 2.0, beta / 2.0);
    }

    void draw_v_mu()
    {
        for (int k = 0; k != (int)fm->v.size(); ++k) {
            double mean = 0.0;
            for (unsigned long p = 0; p != (unsigned long)fm->w.size(); ++p) {
                mean += fm->v[k][p];
            }
            mean /= 1.0 + fm->w.size();
            double var = 1.0 / ((1.0 + fm->w.size()) * v_lambda[k]);
            v_mu[k] = ran_gaussian(mean, std::sqrt(var));
        }
    }

    void draw_v_lambda()
    {
        for (int k = 0; k != (int)fm->v.size(); ++k) {
            double beta = v_mu[k] * v_mu[k] + 1.0;
            for (unsigned long p = 0; p != (unsigned long)fm->w.size(); ++p) {
                beta += (fm->v[k][p] - v_mu[k]) * (fm->v[k][p] - v_mu[k]);
            }
            double alp = 2.0 + fm->w.size();
            v_lambda[k] = ran_gamma(alp / 2.0, beta / 2.0);
        }
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
};

}

#endif
