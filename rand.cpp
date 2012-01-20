#include "rand.h"
#include <QDebug>

namespace Langmuir
{

Random::Random(int seed, QObject *parent) : QObject(parent)
{
    QString string;
    QTextStream stream(&string);
    stream << this->metaObject()->className() << "(" << this << ")";
    setObjectName(string);

    m_seed = 0;
    if(seed <= 0)
    {
        m_seed = static_cast < unsigned int >(time(0));
    }
    else
    {
        m_seed = static_cast < unsigned int >(seed);
    }
    twister = new boost::mt19937(m_seed);

    boost::uniform_01< double > distribution;
    generator01 = new boost::variate_generator < boost::mt19937 &, boost::uniform_01 < double > >(*twister, distribution);
}

Random::~Random()
{
    delete twister;
    delete generator01;
}

unsigned int Random::seed()
{
    return m_seed;
}

double Random::random()
{
    return(*generator01)();
}

double Random::range(const double low, const double high)
{
    boost::uniform_real < double > distribution(low, high);
    boost::variate_generator < boost::mt19937 &, boost::uniform_real < double > >generator(*twister, distribution);
    return generator();
}

double Random::normal(const double mean, const double sigma)
{
    boost::normal_distribution<double> distribution(mean, sigma);
    boost::variate_generator < boost::mt19937 &, boost::normal_distribution < double > >  generator(*twister, distribution);
    return generator();
}

int Random::integer(const int low, const int high)
{
    boost::uniform_int < int > distribution(low, high);
    boost::variate_generator < boost::mt19937 &, boost::uniform_int < int > >generator(*twister, distribution);
    return generator();
}

void Random::testInteger(const int low, const int high, int tries)
{
    for(int i = 0; i < tries; i++)
    {
        int generated = this->integer(low, high);
        Q_ASSERT_X(generated >= low && generated <= high, "testInteger", qPrintable(QString("low: %1 high: %2 generated: %3").arg(low).arg(high).arg(generated)) );
    }
}

bool Random::randomlyChooseYesWithMetropolis(double energyChange, double inversekT)
{
    double randNumber = this->random();
    return randomlyChooseYesWithMetropolis(energyChange, inversekT, randNumber);
}

bool Random::randomlyChooseYesWithMetropolis(double energyChange, double inversekT, double randNumber)
{
    if(energyChange > 0.0)
    {
        if(exp(-energyChange * inversekT)> randNumber)
        {
            return true;
        }
    }
    else
    {
        return true;
    }
    return false;
}

bool Random::randomlyChooseYesWithMetropolisAndCoupling(double energyChange, double inversekT, double coupling)
{
    double randNumber = this->random();
    return randomlyChooseYesWithMetropolisAndCoupling(energyChange, inversekT, coupling, randNumber);
}

bool Random::randomlyChooseYesWithMetropolisAndCoupling(double energyChange, double inversekT, double coupling, double randNumber)
{
    if(energyChange > 0.0)
    {
        if(coupling * exp(-energyChange * inversekT)> randNumber)
        {
            return true;
        }
    }
    else if(coupling > randNumber)
    {
        return true;
    }
    return false;
}

bool Random::randomlyChooseNoWithPercent(double percent)
{
    double randNumber = this->random();
    return randomlyChooseNoWithPercent(percent, randNumber);
}

bool Random::randomlyChooseNoWithPercent(double percent, double randNumber)
{
    if(percent < randNumber)
    {
        return true;
    }
    return false;
}

bool Random::randomlyChooseYesWithPercent(double percent)
{
    double randNumber = this->random();
    return randomlyChooseYesWithPercent(percent, randNumber);
}

bool Random::randomlyChooseYesWithPercent(double percent, double randNumber)
{
    if(percent > randNumber)
    {
        return true;
    }
    return false;
}

}
