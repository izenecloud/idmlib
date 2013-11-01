/*
 * t_ctr.cpp
 *
 *  Created on: Oct 24, 2013
 *      Author:  lucas
 */

#include <idmlib/ctr/AdPredictor.hpp>
#include <idmlib/ctr/FTRL.hpp>

using namespace idmlib;

void test1()
{
    AdPredictor a(0.0, 400.0, 450.0, 0.08);

    std::vector<std::pair<std::string, std::string> > v;
    v.push_back(std::make_pair("age", "3"));
    v.push_back(std::make_pair("gender", "male"));
    v.push_back(std::make_pair("geo", "shanghai"));
    v.push_back(std::make_pair("system", "linux"));
    a.update(v, true);
    a.update(v, false);
    a.update(v, false);
    std::cout << "res: "<< a.predict(v) << std::endl;

    AdPredictor b(a);
    b.forget();
    std::cout << "res: "<< b.predict(v) << std::endl;

    FTRL c(0.5, 1.0, 0.1, 0.1);
    c.update(v, true);
    c.update(v, false);
    c.update(v, false);
    std::cout << "res: "<< c.predict(v) << std::endl;
}

int main()
{
	test1();
}
