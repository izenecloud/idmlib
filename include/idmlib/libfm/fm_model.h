#ifndef FM_MODEL_H_
#define FM_MODEL_H_

#include <utility>
#include <vector>
#include "random.h"

namespace idmlib {

class fm_model {
public:
    double w0;
    std::vector<double> w;
    std::vector<std::vector<double> > v;

    fm_model(unsigned long num_feature, int num_factor, double init_mean, double init_stdev)
        : w0(0.0),
          w(num_feature, 0.0),
          v(num_factor)
    {
        for (int i = 0; i != num_factor; ++i) {
            v[i].reserve(num_feature);
            for (unsigned long j = 0; j != num_feature; ++j) {
                v[i].push_back(ran_gaussian(init_mean, init_stdev));
            }
        }
    }

    double predict(std::vector<std::pair<unsigned long, double> > & row, std::vector<double> & sum, std::vector<double> & sum_sqr)
    {
        double result = 0.0;

        result += w0;

        for (int i = 0; i != (int)row.size(); ++i) {
            result += w[row[i].first] * row[i].second;
        }

        for (int i = 0; i != (int)v.size(); ++i) {
            sum[i] = 0.0;
            sum_sqr[i] = 0.0;
            for (int j = 0; j != (int)row.size(); ++j) {
                double d = v[i][row[j].first] * row[j].second;
                sum[i] += d;
                sum_sqr[i] += d * d;
            }
            result += 0.5 * (sum[i] * sum[i] - sum_sqr[i]);
        }

        return result;
    }
};

}

#endif
