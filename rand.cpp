#include "rand.h"

#include <cstdlib>
#include <cmath>
#include <ctime>
#include <vector>

#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>

using namespace std;

namespace Langmuir{

  Rand::Rand()
  {
    m_gen = new baseGenerator(42u);
    m_dist = new boost::uniform_real<>(0, 1);
    m_uni = new boost::variate_generator<baseGenerator&, boost::uniform_real<> >(*m_gen, *m_dist);
  }

  Rand::Rand(double min, double max)
  {
    // Produce a random number distribution between min and max inclusive
    m_gen = new baseGenerator(42u);
    m_gen->seed(static_cast<unsigned int>(std::time(0)));
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

} // End namespace Langmuir
