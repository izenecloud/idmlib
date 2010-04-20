
#include <idmlib/duplicate-detection/BroderAlgorithm.h>
#include <idmlib/duplicate-detection/NearDuplicateSignature.h>
#include <idmlib/duplicate-detection/IntergerHashFunction.h>
#include <idmlib/duplicate-detection/GetToken.h>

/********************************************************************************
  Description: See "Finding Near-Duplicate Web Pages: A Large-Scale Evaluation
                 of Algorithms" by Monika Henzinger for detail algorithm for
                 computing Broder's document signature.
               This class implements the exact algorithm from the paper.
  Comments   :
  History    : Yeogirl Yun                                        1/22/07
                 Initial Revision
********************************************************************************/
namespace sf1v5{

void BroderAlgorithm::set_fingerprinters() {
    uint64_t id = 0;
    FingerPrinter::reset_rand();
    for(int i = 0; i < m; ++i,++id) {
        fpFuncs[i].set_seed();
    }
    fpSuperShingle.set_seed();
}

/********************************************************************************
  Description: Generates a broder document signature as specified in the paper.
  Comments   :
********************************************************************************/
void BroderAlgorithm::
generate_document_signature(const wiselib::DynamicArray<uint64_t>& docTokens,
			    wiselib::CBitArray& bitArray)
{
  bitArray.SetLength(num_dimensions()/8);

  // first generate n-k+1 number of shingles from a sequence of tokens.
  // using m different fingerprint functions.
  // take the min value of all m fingerprints for each i of n-k+1 shingles
  const int n = docTokens.size();
  //@@ this has an error, not compiling... don't know why. uint64_t is a 8 byte storage and it should be able to assign 8byte value.
  // Array<uint64_t> minShingles(0, m, 0xFFFFFFFFFFFFFFFFL);
  //Array<uint64_t> minShingles(0, m, 0xFFFFFFFFL); // this creates a bug... just to make it compile for now.
  wiselib::Array<uint64_t> minShingles(0, m, std::numeric_limits<uint64_t>::max()); // this creates a bug... just to make it compile for now.
  for (int i = 0; i < n - k + 1; i++) {
    for (int j = i; j < i+k; j++)
      ktokens[j-i] = docTokens[j];

    for (int j = 0; j < m; j++)
      minShingles[j] = min(fpFuncs[j].fp((const char*)&ktokens[0], k*sizeof(uint64_t)), minShingles[j]);
  }

  // now m minshingles are reduced to mPrime shingles using yet-another fingerprint
  // function of fpSuperShingle.
  for (int i = 0; i < mPrime; i++) {
    const int span = i*mPrime;
    for (int j = span; j < span + l; j++)
      ltokens[j-span] = minShingles[j];

    uint64_t supershingle = fpSuperShingle.fp((const char*)&ltokens[0], l*sizeof(uint64_t));

    // set the bits by each supershingle
    wiselib::CBitArray temp;
    temp.Init((uint8_t*)&supershingle, 8);
    int nCount = temp.GetCount();
    for (int nIndex = 0; nIndex < nCount; nIndex++)
      bitArray.SetAt(i*64 + temp.GetIndexBit(nIndex));
  }
}

void BroderAlgorithm::generate_document_signature(const std::vector<unsigned int>& docTokens, wiselib::CBitArray& bitArray) {
    bitArray.SetLength(num_dimensions()/8);
    const int n = docTokens.size();
    wiselib::Array<uint64_t> minShingles(0, m, std::numeric_limits<uint64_t>::max());
    for (int i = 0; i < n - k + 1; i++) {
        for (int j = i; j < i+k; j++) {
            ktokens[j-i] = docTokens[j];
        }

        for (int j = 0; j < m; j++) {
            minShingles[j] = min(fpFuncs[j].fp((const char*)&ktokens[0], k*sizeof(uint64_t)), minShingles[j]);
        }
    }

    for (int i = 0; i < mPrime; i++) {
        const int span = i*mPrime;
        for (int j = span; j < span + l; j++) {
            ltokens[j-span] = minShingles[j];
        }

        uint64_t supershingle = fpSuperShingle.fp((const char*)&ltokens[0], l*sizeof(uint64_t));

        wiselib::CBitArray temp;
        temp.Init((uint8_t*)&supershingle, 8);
        int nCount = temp.GetCount();
        for (int nIndex = 0; nIndex < nCount; nIndex++) {
            bitArray.SetAt(i*64 + temp.GetIndexBit(nIndex));
        }
    }
}

void BroderAlgorithm::generate_document_signature(const std::vector<std::string>& docTokens, wiselib::CBitArray& bitArray)
{  
    bitArray.SetLength(num_dimensions()/8);
    const int n = docTokens.size();
    
    wiselib::Array<uint64_t> minShingles(0, m, std::numeric_limits<uint64_t>::max());
    
    for (int i = 0; i < n - k + 1; i++) {
      std::string ss;
        for (int j = i; j < i+k; j++) {
            ss += docTokens[j];
        }
        
        for (int j = 0; j < m; j++) {
          minShingles[j] = min(fpFuncs[j].fp(ss.c_str(), ss.length()), minShingles[j]);
        }
    }

    for (int i = 0; i < mPrime; i++) {
        const int span = i*mPrime;
        for (int j = span; j < span + l; j++) {
            ltokens[j-span] = minShingles[j];
        }

        uint64_t supershingle = fpSuperShingle.fp((const char*)&ltokens[0], l*sizeof(uint64_t));

        wiselib::CBitArray temp;
        temp.Init((uint8_t*)&supershingle, 8);
        int nCount = temp.GetCount();
        for (int nIndex = 0; nIndex < nCount; nIndex++) {
            bitArray.SetAt(i*64 + temp.GetIndexBit(nIndex));
        }
    }
}

/********************************************************************************
  Description: Returns the number of the same supershingles in the supershingles
                 vector.
  Comments   :
********************************************************************************/
int BroderAlgorithm::neardup_score(NearDuplicateSignature& sig1,
				   NearDuplicateSignature& sig2)
{
  uint64_t* ptr1 = (uint64_t*)sig1.get_bitarray().GetBuffer();
  uint64_t* ptr2 = (uint64_t*)sig2.get_bitarray().GetBuffer();
  int cnt = 0;
  for (int i = 0; i < mPrime; i++) {
    if (ptr1[i] == ptr2[i])
      cnt++;
  }

  return cnt;
}

int BroderAlgorithm::neardup_score(const wiselib::CBitArray& sig1, const wiselib::CBitArray& sig2) {
    uint64_t* ptr1 = (uint64_t*)sig1.GetBuffer();
    uint64_t* ptr2 = (uint64_t*)sig2.GetBuffer();
    int cnt = 0;
    for (int i = 0; i < mPrime; i++) {
        if (ptr1[i] == ptr2[i])
            cnt++;
    }

    return cnt;

}

void BroderAlgorithm::generate_document_signature(const wiselib::YString& document, wiselib::CBitArray& bitArray) {
    bitArray.SetLength(num_dimensions()/8);
    GetToken gt(document);
    uint64_t token_number = 0;
    for(int i = 1; i < k; ++i) {
        if(gt.get_token(token_number)) {
            ktokens[i] = token_number;
        } else {
            break;
        }
    }
    wiselib::Array<uint64_t> minShingles(0, m, std::numeric_limits<uint64_t>::max());
    while(gt.get_token(token_number)) {
        for(int j = 1; j < k; ++j) {
            ktokens[j - 1] = ktokens[j];
        }
        ktokens[k - 1] = token_number;
        for(int j = 0; j < m; j++) {
            uint64_t shingle_j = fpFuncs[j].fp((const char*)&ktokens[0], k*sizeof(uint64_t));
            minShingles[j] = min(shingle_j, minShingles[j]);
        }
    }

    for (int i = 0; i < mPrime; i++) {
        const int span = i*mPrime;
        for (int j = span; j < span + l; j++) {
            ltokens[j-span] = minShingles[j];
        }

        uint64_t supershingle = fpSuperShingle.fp((const char*)&ltokens[0], l*sizeof(uint64_t));

        wiselib::CBitArray temp;
        temp.Init((uint8_t*)&supershingle, 8);
        int nCount = temp.GetCount();
        for (int nIndex = 0; nIndex < nCount; nIndex++) {
            bitArray.SetAt(i*64 + temp.GetIndexBit(nIndex));
        }
    }
}

}

