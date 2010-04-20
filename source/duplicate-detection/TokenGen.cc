
#include <idmlib/duplicate-detection/TokenGen.h>
#include <idmlib/duplicate-detection/NumTokens.h>
//#include "libstemmer_c/include/libstemmer.h"
#include <wiselib/CBitArray.h>
using namespace wiselib;
using namespace sf1v5;
// define RabinFP object dynamically run-time
//FingerPrinter TokenGen::fprinter(2654435761UL);
FingerPrinter TokenGen::fprinter;

#ifdef DO_STEM
static bool isascii(const char *s, ub4 size) {
    for(ub4 i = 0; i < size; ++i) {
        if (s[i] > 'z' || s[i] < 'a') {
            return false;
        }
    }
    return true;
}
#endif

static inline void lower(char * str, int l) {
    for(;l > 0; --l, ++str) *str = tolower(*str);
}

ub8 TokenGen::mfgenerate_token(const char* str, int l) {
#ifdef DO_CASE_CHANGE
    char * s = new char[l];
    memcpy(s, str,l);
    lower(s,l);
    ub8 result = generate_token(s, l);
    delete[] s;
    return result;
#else
    return generate_token(str, l);
#endif//#ifdef DO_CASE_CHANGE
}

/********************************************************************************
  Description: Generate fingerprint tokens from a document. fingerprint size is
                 fixed to be 64 bit, 8 bytes.
  Comments   : //@@ it must be modifed to support Japanese characters.
********************************************************************************/
void TokenGen::generate_tokens_from_document(const YString& document,
					     DynamicArray<ub8>& fpTokens)
{
  DynamicArray<YString> stringTokens;
  document.make_tokens("\n\r\t ", stringTokens, true);
#if defined(DO_STEM) || defined(DO_CASE_CHANGE)
#ifdef DO_STEM
  struct sb_stemmer * stemmer = sb_stemmer_new("english", NULL);
#endif//#ifdef DO_STEM
  for(int i = 0; i < stringTokens.size(); ++i) {
#ifdef DO_CASE_CHANGE
      stringTokens[i].to_lower();
#endif//#ifdef DO_CASE_CHANGE
#ifdef DO_STEM
      if(isascii(&stringTokens[i][0], stringTokens[i].size()) ) {
          char * t = (char * )sb_stemmer_stem(stemmer, (const sb_symbol*)stringTokens[i].c_str(), stringTokens[i].size());
          stringTokens[i] = t;
      }
#endif// #ifdef DO_STEM
  }
#ifdef DO_STEM
  sb_stemmer_delete(stemmer);
#endif//#ifdef DO_STEM

#endif//#if defined(DO_STEM) || defined(DO_CASE_CHANGE)

  for (int i = stringTokens.high(); i >= 0; i--) {
    fpTokens[i] = generate_token(stringTokens[i].c_str(), stringTokens[i].size());
    //fpTokens[i] = NumTokens::get_id((const unsigned char *)stringTokens[i].c_str(), stringTokens[i].size());
  }
}

void TokenGen::generate_tokens_from_document(const YString& document,
        std::vector<unsigned int>& tokens) {
    DynamicArray<YString> stringTokens;
    document.make_tokens("\n\r\t ", stringTokens, true);
#if defined(DO_STEM) || defined(DO_CASE_CHANGE)
#ifdef DO_STEM
    struct sb_stemmer * stemmer = sb_stemmer_new("english", NULL);
#endif//#ifdef DO_STEM
    for(int i = 0; i < stringTokens.size(); ++i) {
#ifdef DO_CASE_CHANGE
        stringTokens[i].to_lower();
#endif//#ifdef DO_CASE_CHANGE
#ifdef DO_STEM
        if(isascii(&stringTokens[i][0], stringTokens[i].size()) ) {
            char * t = (char * )sb_stemmer_stem(stemmer, (const sb_symbol*)stringTokens[i].c_str(), stringTokens[i].size());
            stringTokens[i] = t;
        }
#endif// #ifdef DO_STEM
    }
#ifdef DO_STEM
    sb_stemmer_delete(stemmer);
#endif//#ifdef DO_STEM

#endif//#if defined(DO_STEM) || defined(DO_CASE_CHANGE)

    for (int i = stringTokens.high(); i >= 0; i--) {
        //fpTokens[i] = generate_token(stringTokens[i].c_str(), stringTokens[i].size());
        unsigned int t_id = NumTokens::get_id((const unsigned char *)stringTokens[i].c_str(), stringTokens[i].size());
        tokens.push_back(t_id);
    }

}
