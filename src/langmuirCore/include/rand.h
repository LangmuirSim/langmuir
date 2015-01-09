#ifndef _RAND_H
#define _RAND_H

#include <QObject>
#include <QDataStream>
#include <QTextStream>

#ifndef Q_MOC_RUN

#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)
#pragma GCC diagnostic push
#pragma GCC system_header
#endif

#include <boost/random.hpp>

#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)
#pragma GCC diagnostic pop
#endif

#endif

#include <ctime>

namespace LangmuirCore
{

/**
 * @brief A class to generate random numbers
 */
class Random : public QObject
{
private:
    Q_OBJECT
    Q_DISABLE_COPY(Random)

public:
    /**
     * @brief Random
     * @param seed makes the generator deterministic
     *          - seed == 0 uses the current clock time
     * @param parent object this belongs to
     */
    Random(quint64 seed=0, QObject *parent=0);

    /**
     * @brief Destroy objects
     */
    ~Random();

    /**
     * @brief Get the seed that was used
     */
    quint64 seed();

    /**
     * @brief seed the generator (again)
     *
     * If seed == 0 is used, then the generator \b does \b not use the current time.
     * This is different than the constructor #Random.
     */
    void seed(quint64 seed);

    /**
     * @brief Generate a random double from the uniform distribution [0, 1]
     */
    double random();

    /**
     * @brief Generate a random double from the uniform distribution [low, high]
     */
    double range(const double low=0.0, const double high=1.0);

    /**
     * @brief Generate a random double from the normal distribution
     * @param mean average value of the normal distribution sampled
     * @param sigma standard deviation of the normal distribution sampled
     */
    double normal(const double mean, const double sigma);

    /**
     * @brief Generate a random int from the uniform distribution [low, high]
     */
    int integer(const int low=0, const int high=1);

    /**
     * @brief Randomly choose yes using a Boltzmann factor
     * @param energyChange change in energy when going from initial to final state
     * @param inversekT decay constant in exponential
     */
    bool metropolis(double energyChange, double inversekT);

    /**
     * @brief Randomly choose yes using a Boltzmann factor and coupling constant
     * @param energyChange change in energy when going from initial to final state
     * @param inversekT decay constant in exponential
     * @param coupling alters acceptance probability
     *          - if energy < 0 : chooses yes coupling * Boltzmann factor percent of the time
     *          - if energy > 0 : chooses yes 1 - coupling percent of the time
     */
    bool metropolisWithCoupling(double energyChange, double inversekT, double coupling);

    /**
     * @brief Randomly choose yes a percent of the time
     */
    bool chooseYes(double percent);

    /**
     * @brief Randomly choose no a percent of the time
     */
    bool chooseNo(double percent);

    /**
     * @brief Output the random state to a QDataStream \b Possibly \b Broken
     * @warning This may not quite be working correctly
     */
    friend QDataStream& operator<<(QDataStream& stream, Random& random);

    /**
     * @brief Load the random state from a QDataStream \b Possibly \b Broken
     * @warning This may not quite be working correctly
     */
    friend QDataStream& operator>>(QDataStream& stream, Random& random);

    /**
     * @brief Output the random state to a QTextStream \b Possibly \b Broken
     * @warning This may not quite be working correctly
     */
    friend QTextStream& operator<<(QTextStream& stream, Random& random);

    /**
     * @brief Load the random state from a QTextStream \b Possibly \b Broken
     * @warning This may not quite be working correctly
     */
    friend QTextStream& operator>>(QTextStream& stream, Random& random);

    /**
     * @brief Output the random state to a std::ostream
     */
    friend std::ostream& operator<<(std::ostream& stream, Random& random);

    /**
     * @brief Load the random state from a std::istream
     */
    friend std::istream& operator>>(std::istream& stream, Random& random);

private:
    /**
     * @brief The underlying random number generator
     */
    boost::mt19937 *twister;

    /**
     * @brief The underlying generator coupled to the uniform distribution on [0,1]
     */
    boost::variate_generator<boost::mt19937&, boost::uniform_01<double> >* generator01;

    /**
     * @brief The seed used to start the generator
     */
    quint64 m_seed;
};

}
#endif
