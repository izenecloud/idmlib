
#include <idmlib/duplicate-detection/RandomProjectionEngine.h>
#include <idmlib/duplicate-detection/IntergerHashFunction.h>
#include <idmlib/duplicate-detection/TokenGen.h>

namespace sf1v5{
using namespace std;
/********************************************************************************
  Description: RandomeProjectionEngine projects given a text/string or a token --
                 token is a Rabin's 8byte fingerprint of a text/string -- into
                 n dimensional vector. Each dimension is represented by a real number
                 ranging from -1 to 1. The association from text/token to random
                 projection vector is maintained in a hash table(EfficientLHT).
                 Each dimension is represented by float instead of double
                 to save space.
  Comments   : For details, see "Finding Near-Duplicate Web Pages: A Large-Scale
                 Evaluation of Algorithms" by Monika Henzinger.
               For further technical details, see "Similarity Estimation Techniques
                 from Rounding Algorithms" by Moses S. Charikar.
  History    : Yeogirl Yun                                       1/12/07
                 Intial Revision
********************************************************************************/


// these are static to be shared by all RandomProjectEngine objects.
boost::minstd_rand RandomProjectionEngine::rangen(47u); // random number generator
boost::uniform_real<> RandomProjectionEngine::uni_real(-1., 1.); // uniform distribution in [-1,1]
boost::variate_generator<boost::minstd_rand&, boost::uniform_real<> > RandomProjectionEngine::U(rangen, uni_real);

/********************************************************************************
  Description: Given a token, get its random projection. First lookup the table
                 if it was already built. If no cache found, it builds one and
                 stores in the table and returns its reference.
  Comments   :
********************************************************************************/
const RandomProjection& RandomProjectionEngine::get_random_projection(uint64_t token)
{
#if 0
    const RandomProjection* rp = table.find(token);
    if (rp) {
        return *rp; // found return its reference
    } else { // build a new random projection for this token.
        RandomProjection projection(token, nDimensions);
        for (int i = 0; i < nDimensions; i++) {
            projection[i] = U();
        }
        // now insert this into table.
        table.insert(projection);
        // we must return the reference to the object stored in hash table.
        // so we call find() again.
        const RandomProjection* r = table.find(token);
        ASSERT(r);
        return *r;
    }
#else
    const RandomProjection* rp = table.find(token);
    if(rp) {
        return *rp;
    } else {
        RandomProjection projection(token, nDimensions);
        uint64_t key = token;
        int n = nDimensions - 1;
        float fva[] = {-0.1f, 0.1f};
        while(n >= 0) {
            uint64_t tkey = key;
            for (int i = 0; i < 64 && n >= 0; ++i) {
                projection[n] = fva[tkey & 0x01];
                tkey >>= 1;
                --n;
            }
            if(n >= 0) {
                key = int_hash::hash64shift(key);
            }
        }
        table.insert(projection);
        const RandomProjection * r = table.find(token);
        ASSERT(r);
        /*
        if(token >= 1 && token <= 10 ) {
            cout<<"<"<<token<<": ";
            for(int i = 0; i < nDimensions; ++i) {
                cout<<projection[i]<<"\t";
            }
            cout<<">";
            cout<<endl;
        }*/
        return *r;
    }
    cout<<endl;
#endif
    }

const RandomProjection& RandomProjectionEngine::get_random_projection(const std::string& token)
{
	uint64_t convkey=TokenGen::mfgenerate_token(token.c_str(),token.size());
	return get_random_projection(convkey);
}

}


