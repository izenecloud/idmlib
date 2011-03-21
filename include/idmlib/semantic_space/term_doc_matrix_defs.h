/**
 * @file term_doc_matrix_defs.h
 * @author Zhongxia Li
 * @date Mar 15, 2011
 * @brief 
 */
#ifndef TERM_DOC_MATRIX_DEFS_H_
#define TERM_DOC_MATRIX_DEFS_H_

#include <map>
#include <set>
#include <vector>

#include <boost/shared_ptr.hpp>

#include <idmlib/idm_types.h>
#include <3rdparty/am/rde_hashmap/hash_map.h>
#include <glog/logging.h>

NS_IDMLIB_SSP_BEGIN

typedef uint32_t termid_t;
typedef uint32_t docid_t;
typedef uint32_t count_t;
typedef uint32_t index_t;
typedef float weight_t;

const docid_t MAX_DOC_ID = 0xFFFFFFFF;


struct sTermDocUnit
{
	termid_t termid;
	docid_t docid;
	weight_t weight;
};

struct sDocUnit
{
	docid_t docid;
	weight_t tf; // tf or weight
};

struct sTermUnit
{
	termid_t termid;
	weight_t weight;
};

//struct term_docs_map_ {
//	termid_t termid;
//	sDocUnit* psDocUnit;
//};

// termid,docid to matrix index
typedef std::map< termid_t, index_t > termid_index_map;
typedef std::map< docid_t, index_t > docid_index_map;

typedef std::vector< boost::shared_ptr<sDocUnit> > doc_vector;
typedef std::vector< boost::shared_ptr<sTermUnit> > term_vector;

typedef std::vector< doc_vector > term_docs_map; // for testing
//typedef std::vector< term_vector > doc_terms_map;

typedef term_docs_map term_doc_matrix;
//typedef doc_terms_map doc_term_matrix;

NS_IDMLIB_SSP_END

#endif /* TERM_DOC_UNIT_H_ */

