#ifndef TRANSISTOR_H
#define TRANSISTOR_H

#include "simulation.h"

namespace Langmuir
{
    class Transistor : public Simulation
    {
        public:

            Transistor(SimulationParameters * par, int id = 0);

            /**
             * @brief Perform Iterations.
             *
             * Perform so many iterations of the simulation.
             * @param nIterations number of iterations to perform.
             */
            void performIterations(int nIterations);

        private:

            /**
             * @brief inject charges into the system
             * @param nInjections number of injections to perform.
             */
            void performInjections(int nInjections=1);

            /**
             * @brief next simulation Tick.
             */
            void nextTick();

            /**
             * @brief charge agent transport for QtConcurrent.
             */
            static void chargeAgentIterate( ChargeAgent * chargeAgent );

            /**
             * @brief charge agent transport (choose future site) for QtConcurrent.
             */
            static void chargeAgentChooseFuture( ChargeAgent * chargeAgent );

            /**
             * @brief charge agent transport (apply metropolis) for QtConcurrent.
             */
            static void chargeAgentDecideFuture( ChargeAgent * chargeAgent );
    };
} // End namespace Langmuir

#endif // TRANSISTOR_H
