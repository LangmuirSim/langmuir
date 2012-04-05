#ifndef _RAND_H
#define _RAND_H

#include <ctime>
#include <boost/random.hpp>
#include <QObject>
#include <QDataStream>
#include <QTextStream>

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
    friend QDataStream& operator<<(QDataStream& stream, Random& random);
    friend QDataStream& operator>>(QDataStream& stream, Random& random);
    friend QTextStream& operator<<(QTextStream& stream, Random& random);
    friend QTextStream& operator>>(QTextStream& stream, Random& random);

private:
    boost::mt19937 *twister;
    boost::variate_generator<boost::mt19937&, boost::uniform_01<double> >* generator01;
    quint64 m_seed;
};

}
#endif
