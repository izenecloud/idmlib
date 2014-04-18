#ifndef IDMLIB_B5M_NGRAMSYNONYM_H
#define IDMLIB_B5M_NGRAMSYNONYM_H 
#include "b5m_types.h"
#include <boost/regex.hpp>
#include <util/ustring/UString.h>

NS_IDMLIB_B5M_BEGIN

class NgramSynonym{
public:
    NgramSynonym();
    bool Get(const izenelib::util::UString& ngram, std::vector<izenelib::util::UString>& synonym);

private:

private:
    boost::regex gb_regex_;
};
NS_IDMLIB_B5M_END

#endif

