#ifndef SIMULATION_H
#define SIMULATION_H

#include <QObject>

namespace LangmuirCore
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
 * @brief A class to orchestrate the calculation
 */
class Simulation : public QObject
{
private:
    Q_OBJECT
    Q_DISABLE_COPY(Simulation)

public:
    /**
     * @brief Create a Simulation
     * @param world reference to World Object
     * @param parent QObject this belongs to
     */
    Simulation(World &world, QObject *parent = 0);

    /**
     * @brief Destroy the Simulation
     */
    virtual ~Simulation();

    /**
     * @brief simulate for a set number of steps
     * @param nIterations the number of steps to simulate
     */
    virtual void performIterations(int nIterations);

protected:

    /**
     * @brief Recombine holes and electrons (in solarcell simulations only)
     */
    void performRecombinations();

    /**
     * @brief Tell sources to inject charges
     */
    void performInjections();

    /**
     * @brief \b Try to use the sources to keep the number of ChargeAgents balanced
     */
    void balanceCharges();

    /**
     * @brief Remove charges from the simulation
     *
     * Charges are removed only if a DrainAgent sets their removed status to True.
     * This function will also output carrier statistics if output.id.on.delete
     * is set.
     */
    void nextTick();

    /**
     * @brief A method needed to call ChargeAgent::coulombCPU() in parallel
     */
    static void chargeAgentCoulombInteractionQtConcurrentCPU(ChargeAgent * chargeAgent);

    /**
     * @brief A method needed to call ChargeAgent::coulombGPU() in parallel
     *
     * Does not perform GPU calcuations.  The coulomb kernel in OpenCLHelper is used to do that.
     * This function copies the GPU results from OpenCLHelper to each ChargeAgent. It is assumed
     * that the coulomb kernel was launched beforehand.
     */
    static void chargeAgentCoulombInteractionQtConcurrentGPU(ChargeAgent * chargeAgent);

    /**
     * @brief Reference to World object
     */
    World &m_world;
};

}

#endif
