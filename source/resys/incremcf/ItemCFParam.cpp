#include <idmlib/resys/incremcf/ItemCFParam.h>

namespace
{
const std::size_t DEFAULT_CACHE_SIZE = 1 << 20; // 1M bytes
}

NS_IDMLIB_RESYS_BEGIN

ItemCFParam::ItemCFParam(const std::string& dir)
    : dirPath(dir)
    , coVisitPath((dirPath / "covisit").string())
    , coVisitCacheSize(DEFAULT_CACHE_SIZE)
    , simMatrixPath((dirPath / "sim").string())
    , simMatrixCacheSize(DEFAULT_CACHE_SIZE)
    , simNeighborPath((dirPath / "nb.sdb").string())
    , simNeighborTopK(30)
    , visitFreqPath((dirPath / "visit_freq.sdb").string())
{
}

NS_IDMLIB_RESYS_END
