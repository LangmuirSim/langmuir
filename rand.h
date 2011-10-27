#ifndef _RAND_H
#define _RAND_H

#include<ctime>
#include<boost/random.hpp>

namespace Langmuir
{
class Random
{
    public:

        /**
          * @brief Create Mersenne Twister
          *
          * Random number generator based on the boost library Mersenne Twister
          * @param seed the random number generator; if seed is 0 then use the current time
          */
        Random( int seed=0 );

        /**
         * @brief Destructor.
         *
         * Release the random number generator.
         */
       ~Random();

        /**
          * @brief Set random seed
          *
          * Seed or reseed the random number generator; if seed is 0 then use the current time
          */
        void seed( int );

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
          * @return number
          */
        int integer(const int low=0, const int high=1);

    private:

        /**
          * @brief Mersenne Twister
          */
        boost::mt19937 *twister;

        /**
          * @brief Uniform distribution between 0 and 1
          */
        boost::variate_generator<boost::mt19937&, boost::uniform_01<double> >* generator01;
};
}
#endif
