#ifndef IDMLIB_B5M_PRODUCTTERMANALYZER_H
#define IDMLIB_B5M_PRODUCTTERMANALYZER_H

#include <boost/unordered_set.hpp>
#include <sf1common/type_defs.h>
#include <idmlib/util/idm_analyzer.h>



NS_IDMLIB_B5M_BEGIN
class ProductTermAnalyzer
{

    enum TERM_STATUS {NORMAL, IN_BRACKET, IN_MODEL};

public:
    ProductTermAnalyzer(const std::string& cma_path="");

    ~ProductTermAnalyzer();

    void Analyze(const izenelib::util::UString& title, std::vector<std::pair<std::string, double> >& doc_vector);

private:
    double GetWeight_(uint32_t title_length, const izenelib::util::UString& term, char tag);

private:
    idmlib::util::IDMAnalyzer* analyzer_;
    boost::unordered_set<std::string> stop_set_;
};
NS_IDMLIB_B5M_END

#endif
