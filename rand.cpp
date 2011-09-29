#include "rand.h"

#include <cstdlib>
#include <cmath>
#include <ctime>
#include <vector>

#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/random/variate_generator.hpp>

using namespace std;

namespace Langmuir{

  Rand::Rand( int seed )
  {
    m_gen = new baseGenerator(42u);
    if ( seed >= 0 )
     m_gen->seed( seed );
    else
     m_gen->seed(static_cast<int>(std::time(0)));
    m_dist = new boost::uniform_real<>(0, 1);
    m_uni = new boost::variate_generator<baseGenerator&, boost::uniform_real<> >(*m_gen, *m_dist);
  }

  Rand::Rand(double min, double max, int seed )
  {
    // Produce a random number distribution between min and max inclusive
    m_gen = new baseGenerator(42u);
    if ( seed >= 0 )
     m_gen->seed( seed );
    else
     m_gen->seed(static_cast<int>(std::time(0)));
    m_dist = new boost::uniform_real<>(min, max);
    m_uni = new boost::variate_generator<baseGenerator&, boost::uniform_real<> >(*m_gen, *m_dist);
  }

  Rand::~Rand()
  {
    delete m_uni;
    delete m_dist;
    delete m_gen;
  }

  double Rand::number()
  {
    // Generate a random number between min and max...
    return (*m_uni)();
  }

  double Rand::normalNumber(double mean, double sigma)
  {
    boost::normal_distribution<double> norm_dist(mean, sigma);
    boost::variate_generator<baseGenerator&, boost::normal_distribution<double> >  normal_sampler(*m_gen, norm_dist);

    // sample from the distribution
    return normal_sampler();
  }

} // End namespace Langmuir
