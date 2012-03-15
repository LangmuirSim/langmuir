#ifndef _RAND_H
#define _RAND_H

#include<ctime>
#include<boost/random.hpp>
#include<QObject>

namespace Langmuir
{

class Random : public QObject
{
private:
    Q_OBJECT
    Q_DISABLE_COPY(Random)

public:
    Random(quint64 seed=0, QObject *parent=0);
    ~Random();
    quint64 seed();
    double random();
    double range(const double low=0.0, const double high=1.0);
    double normal(const double mean, const double sigma);
    int integer(const int low=0, const int high=1);
    void testInteger(const int low=0, const int high=1, const int tries=100000);
    bool randomlyChooseYesWithMetropolis(double energyChange, double inversekT, double randNumber);
    bool randomlyChooseYesWithMetropolis(double energyChange, double inversekT);
    bool randomlyChooseYesWithMetropolisAndCoupling(double energyChange, double inversekT, double coupling, double randNumber);
    bool randomlyChooseYesWithMetropolisAndCoupling(double energyChange, double inversekT, double coupling);
    bool randomlyChooseYesWithPercent(double percent, double randNumber);
    bool randomlyChooseYesWithPercent(double percent);
    bool randomlyChooseNoWithPercent(double percent, double randNumber);
    bool randomlyChooseNoWithPercent(double percent);

    template<class CharT, class Traits>
    void save(std::basic_ostream<CharT,Traits>& os)
    {
      os << *twister;
    }

    template<class CharT, class Traits>
    void load(std::basic_istream<CharT,Traits>& is)
    {
      is >> *twister;
    }

private:
    boost::mt19937 *twister;
    boost::variate_generator<boost::mt19937&, boost::uniform_01<double> >* generator01;
    quint64 m_seed;
};

}
#endif
