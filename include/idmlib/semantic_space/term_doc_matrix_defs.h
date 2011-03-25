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

#include <glog/logging.h>
#include <idmlib/idm_types.h>
#include <3rdparty/am/rde_hashmap/hash_map.h>
#include <am/matrix/matrix_file_io.h>
#include <am/matrix/matrix_mem_io.h>
#include <am/matrix/sparse_vector.h>

NS_IDMLIB_SSP_BEGIN

typedef uint32_t termid_t;
typedef uint32_t docid_t;
typedef uint32_t count_t;
typedef uint32_t index_t;
typedef float weight_t;

const docid_t MAX_DOC_ID = 0xFFFFFFFF;

#define SPARSE_MATRIX
//#define RESET_MATRIX_INDEX

/// matrix in fixed size (testing) /////////////////////////////////////////////
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

typedef std::vector< boost::shared_ptr<sDocUnit> > doc_vector;
typedef std::vector< boost::shared_ptr<sTermUnit> > term_vector_;

typedef std::vector< doc_vector > term_docs_map; // for testing
//typedef std::vector< term_vector > doc_terms_map;


/// sparse matrix i/o //////////////////////////////////////////////////////////
typedef izenelib::am::SparseVector<weight_t, docid_t> doc_sp_vector;
typedef izenelib::am::SparseVector<weight_t, termid_t> term_sp_vector;

typedef izenelib::am::MatrixFileFLIo<doc_sp_vector, termid_t> term_doc_matrixf;
typedef izenelib::am::MatrixFileVLIo<doc_sp_vector, termid_t> term_doc_matrixv;
typedef izenelib::am::MatrixFileFLIo<term_sp_vector, docid_t> doc_term_matrixf;
typedef izenelib::am::MatrixFileVLIo<term_sp_vector, docid_t> doc_term_matrixv;


#ifdef SPARSE_MATRIX
typedef term_sp_vector term_vector;
typedef term_doc_matrixf term_doc_matrix;
#else
typedef term_vector_ term_vector;
typedef term_docs_map term_doc_matrix;
#endif

/// map termid/docid to matrix index (1, 2, ... ) //////////////////////////////
#ifdef RESET_MATRIX_INDEX
typedef std::map< termid_t, index_t > termid_index_map;
typedef std::map< docid_t, index_t > docid_index_map;

static const count_t MATRIX_INDEX_START = 1; // start from 1
#endif

/// help functions
template <typename SpVecT>
void PrintSparseVec(SpVecT& svec, string& headInfo="Vector")
{
	cout << headInfo << " [ ";
	for (size_t t = 0; t < svec.size(); t++)
	{
		cout << "(" << svec[t]->first << ", " << svec[t]->second << ") ";
	}
	cout << "] " << endl;
}

NS_IDMLIB_SSP_END

#endif /* TERM_DOC_UNIT_H_ */

