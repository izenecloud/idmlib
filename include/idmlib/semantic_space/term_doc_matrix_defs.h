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

NS_IDMLIB_SSP_BEGIN

typedef uint32_t termid_t;
typedef uint32_t docid_t;
typedef uint32_t count_t;
typedef uint32_t index_t;
typedef float weight_t;

const docid_t MAX_DOC_ID = 0xFFFFFFFF;

/**
 * @brief Unit (triple item) of term-doc (word-concept) matrix,
 */
struct sTermDocUnit
{
	termid_t termid;
	docid_t docid;
	weight_t weight;
};

struct sDocUnit
{
	docid_t docid;
	weight_t weight;
};

struct sTermUnit
{
	termid_t termid;
	weight_t weight;
};

typedef std::map<termid_t, index_t> termid_index_map;
typedef std::map<docid_t, index_t> docid_index_map;

typedef std::vector< boost::shared_ptr<sDocUnit> > doc_vector;
typedef std::vector< boost::shared_ptr<sTermUnit> > term_vector;

typedef std::map< docid_t, term_vector > doc_terms_map;
typedef std::map< termid_t, doc_vector > term_docs_map;

NS_IDMLIB_SSP_END

#endif /* TERM_DOC_UNIT_H_ */

