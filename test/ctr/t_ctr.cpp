/*
 * t_ctr.cpp
 *
 *  Created on: Oct 24, 2013
 *      Author:  lucas
 */

#include <idmlib/ctr/AdPredictor.hpp>

using namespace idmlib;

void test1()
{
    AdPredictor a(0.0, 400.0, 450.0);
    std::vector<std::pair<std::string, std::string> > v;
    v.push_back(std::make_pair("age", "3"));
    v.push_back(std::make_pair("gender", "male"));
    v.push_back(std::make_pair("geo", "shanghai"));
    v.push_back(std::make_pair("system", "linux"));
    a.update(v, true);
    a.update(v, false);
    a.update(v, false);
    std::cout << "res: "<< a.predict(v) << std::endl;
}

int main()
{
	test1();
}
