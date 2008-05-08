#include "rand.h"

#include <cstdlib>
#include <cmath>
#include <vector>

#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>

using namespace std;

namespace Langmuir{

typedef boost::minstd_rand baseGenerator;

  Rand::Rand()
  {
    seed();
  }

  Rand::Rand(double min, double max)
  {
    // Produce a random number distribution between min and max inclusive
    baseGenerator m_gen(42u);
    m_min = min;
    m_range = max - min;
    seed();
  }

  double Rand::number()
  {
    // Generate a random number between min and max...
    return m_range * (double(random()) / double(LONG_MAX)) + m_min;
  }

  double Rand::number(double min, double max)
  {
    // Generate a random number between min and max...
    double range = max - min;
    return range * (double(random()) / double(LONG_MAX)) + min;
  }

  void Rand::seed()
  {
  #ifdef MAC
    srandomdev();
  #endif
  }

} // End namespace Langmuir
