/**
   @file RandomProjectionEngine.h
   @author Jinglei
   @date  2009.11.24
 */
#ifndef RandomProjectionEngine_h
#define RandomProjectionEngine_h 1

#include "RandomProjection.h"
#include "TypeKey.h"
#include "TokenGen.h"

#include "boost/random/linear_congruential.hpp"
#include "boost/random/uniform_real.hpp"
#include "boost/random/variate_generator.hpp"

#include <wiselib/basics.h>
#include <wiselib/hash/EfficientLHT.h>
#include <wiselib/IntTypes.h>
#include <string>

namespace sf1v5{
/**
 * @brief Engine for Random Projection class.
 */
class RandomProjectionEngine {
private:
  static boost::minstd_rand rangen;
  static boost::uniform_real<> uni_real;
  static boost::variate_generator<boost::minstd_rand&, boost::uniform_real<> > U;

  wiselib::EfficientLHT<wiselib::TypeKey<wiselib::ub8>, RandomProjection> table; // table lookup from 64bit token to projection

  int nDimensions;
  
public:
  inline RandomProjectionEngine(int n)
    : nDimensions(n) {}
  inline ~RandomProjectionEngine() {}
  inline int num_dimensions() const { return nDimensions; }
  // methods to get a random projection.
  const RandomProjection& get_random_projection(wiselib::ub8 token);
  const RandomProjection& get_random_projection(const std::string& token);
};
}

#endif
