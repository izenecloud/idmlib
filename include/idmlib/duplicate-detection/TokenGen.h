/**
   @file TokenGen.h
   @author Jinglei
   @date 2009.11.24
 */
#ifndef TokenGen_h
#define TokenGen_h 1

#include "FingerPrinter.h"
#include<types.h>
//#include <wiselib/basics.h>
#include <wiselib/YString.h>
#include <wiselib/DArray.h>

#include <vector>
namespace sf1v5{
/**
 * @brief generate fingerprint from a string token.
 */
class TokenGen {
    /**
     * @brief hash class
     */
  static FingerPrinter fprinter;
public:
  /**
   * @brief hash a string to get the token
   *
   * @param str source string
   * @param l source string length
   *
   * @return hash value
   */
  inline static uint64_t generate_token(const char* str, int l) { return fprinter.fp(str, l); }

  /**
   * @brief multifunction hash a string to get the token; include lowercase, maybe stem
   *
   * @param str source
   * @param l source length
   *
   * @return generated hash value
   */
  static uint64_t mfgenerate_token(const char* str, int l);
  /**
   * @brief generate tokens from a document
   * use finger printer hash function
   *
   * @param[in] document content of a document
   * @param[out] tokens generated tokens of the document
   */
  static void generate_tokens_from_document(const wiselib::YString& document,
					    wiselib::DynamicArray<uint64_t>& tokens);
  /**
   * @brief generate dokens from a document
   * use a map
   *
   * @param[in] document content of a document
   * @param[out] tokens generated tokens of the document
   */
  static void generate_tokens_from_document(const wiselib::YString& document,
					    std::vector<unsigned int>& tokens);
};
}
#endif
