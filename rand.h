#ifndef _RAND_H
#define _RAND_H

#include<ctime>
#include<boost/random.hpp>
#include<QObject>

namespace Langmuir
{
/**
  *  @class Random
  *  @brief Interface to boost random number library
  */
class Random : public QObject
{
Q_OBJECT public:

    /**
          * @brief Create Mersenne Twister
          *
          * Random number generator based on the boost library Mersenne Twister
          * @param seed the random number generator; if seed is 0 then use the current time
          */
    Random(int seed=0, QObject *parent=0);

    /**
         * @brief Destructor.
         *
         * Release the random number generator.
         */
    ~Random();

    /**
          * @brief get the seed used when setting up the generator
          */
    unsigned int seed();

    /**
          * @brief Sample random number generator
          *
          * Sample the uniform distribution between 0 and 1
          * @return number
          */
    double random();

    /**
          * @brief Sample random number generator
          *
          * Sample the uniform distribution between low and high for doubles
          * @return number
          */
    double range(const double low=0.0, const double high=1.0);

    /**
          * @brief Sample random number generator
          *
          * Sample the gaussian distribution with a certain mean and standard deviation
          * @return number
          */
    double normal(const double mean, const double sigma);

    /**
          * @brief Sample random number generator
          *
          * Sample the uniform distribution between low and high for integers
          * @note random number range includes low and high
          *       [low, high] = low, low + 1, low + 2 ... high - 2, high - 1, high
          * @return number
          */
    int integer(const int low=0, const int high=1);

    /**
          * @brief Check that random number generator is generating numbers in correct ranges
          */
    void testInteger(const int low=0, const int high=1, const int tries=100000);

    /**
         * @brief Basic Monte Carlo acceptance criterion
         *
         * Apply the metropolis criterion.
         * @param energyChange energy difference for change
         * @param inversekT one over Boltzmann constant
         * @param randNumber an already generated random number
         * @return success
         */
    bool randomlyChooseYesWithMetropolis(double energyChange, double inversekT, double randNumber);

    /**
         * @brief Basic Monte Carlo acceptance criterion
         *
         * this is an overloaded function that generates a random number for you
         * @param energyChange energy difference for change
         * @param inversekT one over Boltzmann constant
         * @return success
         */
    bool randomlyChooseYesWithMetropolis(double energyChange, double inversekT);

    /**
         * @brief Basic Monte Carlo acceptance criterion
         *
         * Apply the metropolis criterion with coupling.
         * @param energyChange energy difference for change
         * @param inversekT one over Boltzmann constant
         * @param coupling coupling constant between old and new site types.
         * @param randNumber an already generated random number
         * @return success
         */
    bool randomlyChooseYesWithMetropolisAndCoupling(double energyChange, double inversekT, double coupling, double randNumber);

    /**
         * @brief Basic Monte Carlo acceptance criterion with coupling
         *
         * this is an overloaded function that generates a random number for you
         * @param energyChange energy difference for change
         * @param inversekT one over Boltzmann constant
         * @param coupling coupling constant between old and new site types.
         * @return success
         */
    bool randomlyChooseYesWithMetropolisAndCoupling(double energyChange, double inversekT, double coupling);

    /**
         * @brief Basic Random Yes Choice
         *
         * returns true about 100*percent times out of every 100 calls
         * @param percent fraction less than or equal to one of the time to return true
         * @param randNumber an already generated random number
         * @return success
         */
    bool randomlyChooseYesWithPercent(double percent, double randNumber);

    /**
         * @brief Basic Random Yes Choice
         *
         * this is an overloaded function for a already generated random number
         * @param percent fraction less than or equal to one of the time to return true
         * @return success
         */
    bool randomlyChooseYesWithPercent(double percent);

    /**
         * @brief Basic Random No Choice
         *
         * returns false about 100*percent times out of every 100 calls
         * @param percent fraction less than or equal to one of the time to return false
         * @param randNumber an already generated random number
         * @return success
         */
    bool randomlyChooseNoWithPercent(double percent, double randNumber);

    /**
         * @brief Basic Random No Choice
         *
         * this is an overloaded function for a already generated random number
         * @param percent fraction less than or equal to one of the time to return false
         * @return success
         */
    bool randomlyChooseNoWithPercent(double percent);

private:

    /**
          * @brief Mersenne Twister
          */
    boost::mt19937 *twister;

    /**
          * @brief Uniform distribution between 0 and 1
          */
    boost::variate_generator<boost::mt19937&, boost::uniform_01<double> >* generator01;

    /**
          * @brief The value of the seed used to create generator
          */
    unsigned int m_seed;
};
}
#endif
