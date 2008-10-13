/*
 *  Rand - our random number generator. Simple interface to the Boost random
 *  number library functions.
 */

#ifndef __RAND_H
#define __RAND_H

#include <boost/random/linear_congruential.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/random/uniform_real.hpp>

typedef boost::minstd_rand baseGenerator;

namespace Langmuir
{

  class Rand
  {
  public:
    Rand();
    Rand(double min, double max);
    ~Rand();
    double number();
//    double number(double min, double max);

  private:
    double m_min;
    double m_range;
    baseGenerator *m_gen;
    boost::uniform_real<> *m_dist;
    boost::variate_generator<baseGenerator&, boost::uniform_real<> > *m_uni;
  };

} // End namespace Langmuir

#endif
