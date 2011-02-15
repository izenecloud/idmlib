#ifndef _IDMTYPES_H
#define _IDMTYPES_H

#ifndef WIN32
#include <stddef.h>
#include <sys/types.h>
#include <stdint.h>
#else
#include <sys/types.h>
#include <wchar.h>
typedef signed char       int8_t;
typedef short             int16_t;
typedef long              int32_t;
typedef __int64           int64_t;
typedef unsigned char     uint8_t;
typedef unsigned short    uint16_t;
typedef unsigned long     uint32_t;
typedef unsigned __int64  uint64_t;

#endif //end of WIN32

// typedef uint32_t termid_t;
// typedef uint32_t docid_t;
// typedef uint32_t count_t;

#define NS_IDMLIB_BEGIN namespace idmlib{
#define NS_IDMLIB_END }

#define NS_IDMLIB_KPE_BEGIN namespace idmlib{ namespace kpe{
#define NS_IDMLIB_KPE_END }}

#define NS_IDMLIB_CC_BEGIN namespace idmlib{ namespace cc{
#define NS_IDMLIB_CC_END }}

#define NS_IDMLIB_WIKI_BEGIN namespace idmlib{ namespace wiki{
#define NS_IDMLIB_WIKI_END }}

#define NS_IDMLIB_UTIL_BEGIN namespace idmlib{ namespace util{
#define NS_IDMLIB_UTIL_END }}

#endif

