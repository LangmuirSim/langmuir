#include "rand.h"

using namespace Langmuir;

Random::Random (int seed)
{
    if (seed <= 0)
    {
        twister = new boost::mt19937 (static_cast < unsigned int >(time (0)));
    }
    else
    {
        twister = new boost::mt19937 (static_cast < unsigned int >(seed));
    }

    boost::uniform_01< double > distribution;
    generator01 = new boost::variate_generator < boost::mt19937 &, boost::uniform_01 < double > >(*twister, distribution);
}

Random::~Random ()
{
    delete twister;
    delete generator01;
}

void Random::seed (int seed)
{
    twister->seed (static_cast < unsigned int >(seed));
}

double Random::random ()
{
    return (*generator01) ();
}

double Random::range (const double low, const double high)
{
    boost::uniform_real < double > distribution(low, high);
    boost::variate_generator < boost::mt19937 &, boost::uniform_real < double > >generator (*twister, distribution);
    return generator ();
}

double Random::normal(const double mean, const double sigma)
{
    boost::normal_distribution<double> distribution(mean, sigma);
    boost::variate_generator < boost::mt19937 &, boost::normal_distribution < double > >  generator(*twister, distribution);
    return generator();
}

int Random::integer (const int low, const int high)
{
    boost::uniform_int < int > distribution(low, high);
    boost::variate_generator < boost::mt19937 &, boost::uniform_int < int > >generator (*twister, distribution);
    return generator();
}
