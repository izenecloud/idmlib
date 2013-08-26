#ifndef SPARSE_DATA_H_
#define SPARSE_DATA_H_

#include <utility>
#include <vector>
#include <cmath>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <limits>

namespace idmlib {

class sparse_data {
public:
    std::vector<std::vector<std::pair<unsigned long, double> > > X;
    std::vector<std::vector<std::pair<unsigned long, double> > > X_t;
    std::vector<double> Y;
    double minY;
    double maxY;
    unsigned long num_feature;

    sparse_data(const std::string & filename)
    {
        std::ifstream fData(filename.c_str());
        if (!fData) {
            throw "unable to open " + filename;
        }

        minY = +std::numeric_limits<double>::max();
        maxY = -std::numeric_limits<double>::max();
        num_feature = 0;

        std::string line;
        while (std::getline(fData, line)) {
            std::istringstream in(line);

            double target;
            in >> target;
            minY = std::min(target, minY);
            maxY = std::max(target, maxY);
            Y.push_back(target);

            std::vector<std::pair<unsigned long, double> > row;
            unsigned long c;
            double v;
            while (true) {
                in >> c >> v;
                if (!in) {
                    break;
                }
                num_feature = std::max(c, num_feature);
                row.push_back(std::make_pair(c, v));
            }
            X.push_back(row);
        }
        ++num_feature;
        std::cout << filename << " loaded..." << std::endl;
    }

    void transpose()
    {
        X_t.resize(num_feature);
        std::vector<unsigned long> count(num_feature, 0);

        for (unsigned long i = 0; i != (unsigned long)X.size(); ++i) {
            for (int j = 0; j != (int)X[i].size(); ++j) {
                count[X[i][j].first] += 1;
            }
        }

        for (unsigned long i = 0; i != num_feature; ++i) {
            X_t[i].reserve(count[i]);
        }

        for (unsigned long i = 0; i != (unsigned long)X.size(); ++i) {
            for (int j = 0; j != (int)X[i].size(); ++j) {
                X_t[X[i][j].first].push_back(std::make_pair(i, X[i][j].second));
            }
        }
    }
};

}

#endif
