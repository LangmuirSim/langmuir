#ifndef SIMULATION_H
#define SIMULATION_H

#include <QObject>

namespace Langmuir
{
class World;
class Grid;
class Agent;
class Potential;
class DrainAgent;
class SourceAgent;
class ChargeAgent;
struct SimulationParameters;
/**
    *  @class Simulation
    *  @brief Simulation management.
    *
    *  Organizes simulation and performs relevent tasks.
    *  @date 06/07/2010
    */
class Simulation : public QObject
{
private:
    Q_OBJECT
    Q_DISABLE_COPY(Simulation)

public:

    /**
     * @brief Constructor.
     *
     * Set up the basic parameters of the simulation.
     * @param par parameter struct
     */
    Simulation(SimulationParameters * par, QObject *parent = 0);

    /**
     * @brief Destructor.
     *
     * Releases the world object.
     */
    virtual ~Simulation();

    /**
     * @brief Perform Iterations. 
     *
     * Perform so many iterations of the simulation.
     * @param nIterations number of iterations to perform.
     */
    virtual void performIterations(int nIterations);

    /**
     * @brief World access.
     *
     * Retrieve the world object - can be used alter world properties.
     */
    World *world();

protected:

    /**
     * @brief Add Defects
     *
     * Defects are sites that carriers can not transport to
     */
    virtual void createDefects();

    /**
     * @brief Place charges on grid.
     *
     * Seed the grid with the number of charges specified by setMaxCharges.
     */
    virtual void seedCharges();

    /**
     * @brief inject charges into the system
     * @param nInjections number of injections to perform.
     */
    void performInjections();

    /**
     * @brief next simulation Tick.
     */
    void nextTick();

    /**
     * @brief World pointer.
     */
    World *m_world;

    /**
             * @brief charge agent transport for QtConcurrent.
             */
    static void chargeAgentIterate(ChargeAgent * chargeAgent);

    /**
             * @brief charge agent transport(choose future site)for QtConcurrent.
             */
    static void chargeAgentChooseFuture(ChargeAgent * chargeAgent);

    /**
             * @brief charge agent transport(apply metropolis)for QtConcurrent.
             */
    static void chargeAgentDecideFuture(ChargeAgent * chargeAgent);

};

inline World* Simulation::world()
{
    return m_world;
}
}

#endif
