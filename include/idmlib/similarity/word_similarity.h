#ifndef IDMLIB_SIM_WORDSIMILARITY_H_
#define IDMLIB_SIM_WORDSIMILARITY_H_


#include <string>
#include <iostream>
#include <idmlib/idm_types.h>
NS_IDMLIB_SIM_BEGIN

class WordSimilarity
{
public:
  typedef uint32_t WordId;
  typedef boost::function<void (WordId, WordId, double) > FunctionType;
  WordSimilarity(const std::string& dir, FunctionType callback);
  ~WordSimilarity();
  
  bool Open();
  
  bool Flush();
  
  bool Append(WordId id, const std::vector<std::pair<uint32_t, uint32_t>& doc_item);
  
  bool Compute();
  
 private: 
  std::string dir_;
  FunctionType callback_;
};

   
NS_IDMLIB_SIM_END



#endif 
