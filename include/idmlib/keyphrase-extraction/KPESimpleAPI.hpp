/*
 * \file KPESimpleAPI.hpp
 * \brief 
 * \date Apr 27, 2010
 * \author Vernkin Chen
 */

#ifndef KPESIMPLEAPI_HPP_
#define KPESIMPLEAPI_HPP_

#include <idmlib/idm_types.h>
#include <vector>
#include <string>

NS_IDMLIB_KPE_BEGIN

using std::string;

class KPEDataInterator
{
public:
    virtual ~KPEDataInterator()
    {
    }

    /**
     * \param outStr to get the output string
     * \param docId to get the document ID
     * \return true if can get current document
     */
    virtual bool next( string& outStr, uint32_t& docId )
    {
        return false;
    }
};

void perform_kpe( const string& resPath, KPEDataInterator& inputItr,
        std::vector<std::string>& kpeVec, const string idDataPath = "./id",
        const string kpeDataPath = "./tmp");


NS_IDMLIB_KPE_END

#endif /* KPESIMPLEAPI_H_ */
