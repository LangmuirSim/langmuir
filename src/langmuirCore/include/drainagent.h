#ifndef DRAINAGENT_H
#define DRAINAGENT_H

#include "fluxagent.h"

namespace LangmuirCore
{

class ChargeAgent;

/**
 * @brief A class to remove charges
 *
 * Unlike SourceAgents, the algorithms for removing charges currently
 * reside in the Simulation class.  This should be fixed.
 * The DrainAgent is keeping track of drain statistics and
 * transport probability.
 */
class DrainAgent : public FluxAgent
{
public:
    /**
     * @brief create a DrainAgent
     */
    DrainAgent(World &world, Grid &grid, QObject *parent = 0);

    /**
     * @brief accept charge with constant probability
     */
    virtual bool tryToAccept(ChargeAgent *charge);
};

/**
 * @brief A class to remove ElectronAgents
 */
class ElectronDrainAgent : public DrainAgent
{
public:
    /**
     * @brief create an ElectronDrainAgent at a specific site
     */
    ElectronDrainAgent(World &world, int site, QObject *parent = 0);

    /**
     * @brief create a ElectronDrainAgent at a specific Grid::CubeFace
     */
    ElectronDrainAgent(World &world, Grid::CubeFace cubeFace, QObject *parent = 0);
protected:
    /**
     * @brief same as FluxAgent::energyChange(), but specialized for ElectronAgents.
     *
     * Note really used because the default FluxAgent::shouldTransport() behavoir,
     * which is to use a simple constant probability, has not been reimplemented
     * for DrainAgents.
     */
    virtual double energyChange(int fSite);
};

/**
 * @brief A class to remove HoleAgents
 */
class HoleDrainAgent : public DrainAgent
{
public:
    /**
     * @brief create an HoleDrainAgent at a specific site
     */
    HoleDrainAgent(World &world, int site, QObject *parent = 0);

    /**
     * @brief create a HoleDrainAgent at a specific Grid::CubeFace
     */
    HoleDrainAgent(World &world, Grid::CubeFace cubeFace, QObject *parent = 0);
protected:
    /**
     * @brief same as FluxAgent::energyChange(), but specialized for HoleAgents.
     *
     * Note really used because the default FluxAgent::shouldTransport() behavoir,
     * which is to use a simple constant probability, has not been reimplemented
     * for DrainAgents.
     */
    virtual double energyChange(int fSite);
};

/**
 * @brief A class to remove Excitons
 *
 * Currently, the RecombinationAgent is not really doing much of anything outside of
 * keeping track of recombination statistics.  The meat of the recombination resides
 * in Simulation::performRecombinations().  This should probably be changes in the
 * future.
 */
class RecombinationAgent : public DrainAgent
{
public:
    /**
     * @brief create a RecombinationAgent
     */
    RecombinationAgent(World &world, QObject *parent = 0);

    /**
     * @brief accept charge with constant probability
     */
    virtual bool tryToAccept(ChargeAgent *charge);

    /**
     * @brief calculate an acceptance probability based upon the desired rate and encounter frequency
     */
    void guessProbability();

private:
    /**
     * @brief currently implemented as zero and not really used
     */
    virtual double energyChange(int fSite);
};

}
#endif
