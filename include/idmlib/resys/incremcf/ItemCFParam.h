#ifndef IDMLIB_RESYS_ITEM_CF_PARAM_H
#define IDMLIB_RESYS_ITEM_CF_PARAM_H

#include <idmlib/idm_types.h>
#include <string>
#include <boost/filesystem/path.hpp>

NS_IDMLIB_RESYS_BEGIN

struct ItemCFParam
{
    boost::filesystem::path dirPath;

    std::string coVisitPath;
    std::size_t coVisitCacheSize;

    std::string simMatrixPath;
    std::size_t simMatrixCacheSize;

    std::string simNeighborPath;
    std::size_t simNeighborTopK;

    std::string visitFreqPath;

    ItemCFParam(const std::string& dir);
};

NS_IDMLIB_RESYS_END

#endif
