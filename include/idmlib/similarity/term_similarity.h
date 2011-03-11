#ifndef IDMLIB_SIM_TERMSIMILARITY_H_
#define IDMLIB_SIM_TERMSIMILARITY_H_


#include <string>
#include <iostream>
#include <boost/function.hpp>
#include <idmlib/idm_types.h>
#include <idmlib/semantic_space/random_indexing.h>
#include <am/matrix/matrix_file_io.h>
NS_IDMLIB_SIM_BEGIN

class TermSimilarity
{
public:
  typedef uint32_t IdType;
  typedef boost::function<void (IdType, IdType, double) > FunctionType;
  typedef std::vector<uint32_t> VectorType;
  typedef uint32_t ContextIdType;
  typedef uint32_t FreqType;
  typedef idmlib::ssp::RandomIndexing<izenelib::am::MatrixFileFLIo, VectorType, IdType, ContextIdType, FreqType> RIType;
  
  TermSimilarity(const std::string& dir, FunctionType callback);
  ~TermSimilarity();
  
  bool Open();
  
  bool Flush();
  
  bool Append(IdType id, const std::vector<std::pair<ContextIdType, FreqType> >& doc_item);
  
  bool Compute();
  
 private:

 private: 
  std::string dir_;
  FunctionType callback_;
  RIType* ri_;
};

   
NS_IDMLIB_SIM_END



#endif 
