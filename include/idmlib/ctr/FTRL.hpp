#ifndef FTRL_H_
#define FTRL_H_

#include <ir/be_index/AVMapper.hpp>
#include <vector>
#include <cmath>
#include <utility>
#include <string>
#include <types.h>

using namespace izenelib::ir::be_index;

namespace idmlib {

class FTRLweights {
public:
    FTRLweights()
    {
        z = 0.0;
        n = 0.0;
    }

    double w;
    double g;
    double delta;
    double z;
    double n;
};

class FTRL {
public:
    FTRL()
    {
    }

    FTRL(double alpha, double beta, double lambda_1, double lambda_2)
        : alpha(alpha), beta(beta), lambda_1(lambda_1), lambda_2(lambda_2)
    {
    }

    void update(const std::vector<std::pair<std::string, std::string> > & assignment, bool clicked)
    {
        std::vector<std::pair<uint32_t, uint32_t> > pos(assignment.size());
        double sum_of_w = 0.0;

        for (std::size_t i = 0; i != assignment.size(); ++i) {
            std::pair<std::pair<uint32_t, bool>, std::pair<uint32_t, bool> > avResult = avMapper.insert(assignment[i].first, assignment[i].second);
            pos[i].first = avResult.first.first;
            pos[i].second = avResult.second.first;
            bool newAttribute = avResult.first.second;
            bool newValue = avResult.second.second;

            if (newAttribute) {    // new attribute
                weights.resize(weights.size() + 1);
                weights.back().push_back(FTRLweights());
            } else if (newValue) {   // old attribute, but new value
                weights[pos[i].first].push_back(FTRLweights());
            }

            FTRLweights & unit = weights[pos[i].first][pos[i].second];
            getW(unit);
            sum_of_w += unit.w;
        }

        double p = sigmoid(sum_of_w);

        for (std::size_t i = 0; i != assignment.size(); ++i) {
            FTRLweights & unit = weights[pos[i].first][pos[i].second];
            unit.g = p - clicked ? 1.0 : 0.0;
            unit.delta = (sqrt(unit.n + unit.g * unit.g) - sqrt(unit.n)) / alpha;
            unit.z += unit.g - unit.delta * unit.w;
            unit.n += unit.g * unit.g;
        }
    }

    double predict(const std::vector<std::pair<std::string, std::string> > & assignment)
    {
        double sum_of_w = 0.0;

        for (std::size_t i = 0; i != assignment.size(); ++i) {
            std::pair<std::pair<uint32_t, bool>, std::pair<uint32_t, bool> > avResult = avMapper.has(assignment[i].first, assignment[i].second);
            if (!(avResult.first.second == true && avResult.second.second == true)) {
                continue;
            }
            sum_of_w += weights[avResult.first.first][avResult.second.first].w;
        }

        return sigmoid(sum_of_w);
    }

private:
    void getW(FTRLweights & unit)
    {
        if (unit.z <= lambda_1 && unit.z >= -lambda_1) {
            unit.w = 0.0;
            return;
        }
        // otherwise...
        if (unit.z > 0) {
            unit.w = -(unit.z - lambda_1) / ((beta + sqrt(unit.n)) / alpha + lambda_2);
        } else {
            unit.w = -(unit.z + lambda_1) / ((beta + sqrt(unit.n)) / alpha + lambda_2);
        }
    }

    double sigmoid(double x)
    {
        return 1.0 / (1.0 + exp(-x));
    }

    double alpha;
    double beta;
    double lambda_1;
    double lambda_2;

    AVMapper avMapper;
    std::vector<std::vector<FTRLweights> > weights;
};

}

#endif
