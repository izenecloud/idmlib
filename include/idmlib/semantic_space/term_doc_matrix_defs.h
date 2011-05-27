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
#include <sstream>

#include <boost/shared_ptr.hpp>

#include <glog/logging.h>
#include <idmlib/idm_types.h>
#include <idmlib/util/idm_term.h>
#include <idmlib/util/time_checker.h>
#include <3rdparty/am/rde_hashmap/hash_map.h>
#include <am/matrix/matrix_file_io.h>
#include <am/matrix/matrix_mem_io.h>
#include <am/matrix/sparse_vector.h>
#include <util/profiler/ProfilerGroup.h>
#include <ir/index_manager/utility/system.h>
using namespace izenelib::ir::indexmanager;

NS_IDMLIB_SSP_BEGIN

//typedef uint32_t termid_t;
//typedef uint32_t docid_t;
//typedef uint32_t count_t;
typedef uint32_t index_t;
typedef float weight_t;

const docid_t MAX_DOC_ID = 0xFFFFFFFF;

#define SPARSE_MATRIX

/* testing */
//#define SSP_BUIDER_TEST
#ifdef IMD_TIME_CHECKER
  #define SSP_TIME_CHECKER
#endif

//typedef std::vector<idmlib::util::IDMTerm> IdmTermList;
typedef std::map<termid_t, string> IdmTermList;
static IdmTermList NULLTermList; // default parameter value

/// sparse matrix i/o //////////////////////////////////////////////////////////
typedef izenelib::am::SparseVector<weight_t, docid_t> doc_sp_vector;
typedef izenelib::am::SparseVector<weight_t, termid_t> term_sp_vector;
typedef izenelib::am::SparseVector<weight_t, docid_t> interpretation_vector_type;

typedef izenelib::am::MatrixFileFLIo<doc_sp_vector, termid_t> term_doc_matrixf;
typedef izenelib::am::MatrixFileVLIo<doc_sp_vector, termid_t> term_doc_matrixv;
typedef izenelib::am::MatrixFileFLIo<term_sp_vector, docid_t> doc_term_matrixf;
typedef izenelib::am::MatrixFileVLIo<term_sp_vector, docid_t> doc_term_matrixv;

typedef izenelib::am::MatrixFileVLIo<doc_sp_vector, docid_t> doc_doc_matrixv;


typedef std::vector<std::pair<docid_t, weight_t> > InterpretVector;

#ifdef SPARSE_MATRIX
typedef term_sp_vector term_vector;
typedef term_doc_matrixv term_doc_matrix_file_oi;
typedef doc_term_matrixv doc_term_matrix_file_oi;
typedef doc_doc_matrixv doc_doc_matrix_file_io;
#else
typedef term_vector_ term_vector;
typedef term_docs_map term_doc_matrix;
#endif


/// help functions
struct sort_second {
    bool operator()(const std::pair<uint32_t, weight_t> &left, const std::pair<uint32_t, weight_t> &right) {
        return left.second > right.second;
    }
};

template <typename SpVecT>
void PrintSparseVec(SpVecT& svec, const string& headInfo=string("Vector"))
{
	cout << headInfo << " len:" << svec.value.size() << " [ ";
	for (size_t t = 0; t < svec.value.size(); t++)
	{
		cout << "(" << svec.value.at(t).first << ", " << svec.value.at(t).second << ") ";
	}
	cout << "] " << endl;
}


NS_IDMLIB_SSP_END

#endif /* TERM_DOC_UNIT_H_ */

