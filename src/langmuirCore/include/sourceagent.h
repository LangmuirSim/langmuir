#ifndef SOURCEAGENT_H
#define SOURCEAGENT_H

#include "fluxagent.h"

namespace LangmuirCore
{

/**
 * @brief A class to inject charges
 */
class SourceAgent : public FluxAgent
{
public:
    /**
     * @brief create a SourceAgent
     */
    SourceAgent(World &world, Grid &grid, QObject *parent = 0);

    /**
     * @brief seed a charge at a random site
     * @warning does not call shouldTransport()
     *
     * Attempts to seed a charge at a random site, without calling shouldTransport().
     * However, validToInject() is still called.  This function is used when
     * randomly placing charges in the system.
     */
    bool tryToSeed();

    /**
     * @brief seed a charge at a \b specific site
     * @warning does not call shouldTransport()
     *
     * Attempts to seed a charge at a specific site, without calling shouldTransport().
     * However, validToInject() is still called.  This function is used when
     * placing charges at specific places.  For example, when sometimes the checkpoint
     * file has information on where charges are/were, and these need to be placed.
     */
    bool tryToSeed(int site);

    /**
     * @brief attempt to inject a carrier
     *
     * This is the main transport method of a SourceAgent.  This function uses
     * chooseSite(), shouldTransport() and validToInject() to inject the charge.
     * It is not garunteed that a charge will be injected.
     */
    bool tryToInject();

protected:
    /**
     * @brief choose a site to inject to
     *
     * By default, choose a site from the SourceAgent's neighborlist.
     * The Grid::CubeFace used to construct the SourceAgent determines the
     * neighborlist.
     */
    virtual int chooseSite();

    /**
     * @brief checks to see if a carrier can actually be injected at the requested site
     *
     * For example, if the site contains a defect, or a carrier is already present at the
     * site, then it is not valid to inject the carrier at this site.
     */
    virtual bool validToInject(int site)= 0;

    /**
     * @brief actually injects carrier.
     * @warning this function assumes that injecting a charge at the requested site is allowed
     *
     * Creates a new carrier.  Does not perform checks.  Forcefully injects charge.
     * Don't call this function unless you know what you are doing.
     */
    virtual void inject(int site)= 0;

    /**
     * @brief decides if charge should be injected using a constant probability
     * @param site the site involved
     *
     * If SimulationParameters::sourceMetropolis is true, then use the metropolis
     * criterion with an energy change to decide if charge should be injected.
     */
    virtual bool shouldTransport(int site);

    /**
     * @brief choose a random site ID
     *
     * It can be any possible site in the grid.
     */
    int randomSiteID();

    /**
     * @brief choose a random site ID from the neighborlist.
     */
    int randomNeighborSiteID();
};

/**
 * @brief A class to inject ElectronAgents
 */
class ElectronSourceAgent : public SourceAgent
{
public:
    /**
     * @brief create an ElectronSourceAgent at a specific site
     */
    ElectronSourceAgent(World &world, int site, QObject *parent = 0);

    /**
     * @brief create an ElectronSourceAgent at a specific Grid::CubeFace
     */
    ElectronSourceAgent(World &world, Grid::CubeFace cubeFace, QObject *parent = 0);
protected:
    /**
     * @brief same as SourceAgent::validToInject(), but specialized for ElectronAgents.
     */
    virtual bool validToInject(int site);

    /**
     * @brief same as FluxAgent::energyChange(), but specialized for ElectronAgents.
     */
    virtual double energyChange(int site);

    /**
     * @brief same as SourceAgent::inject(), but specialized for ElectronAgents.
     */
    virtual void inject(int site);
};

/**
 * @brief A class to inject HoleAgents
 */
class HoleSourceAgent : public SourceAgent
{
public:
    /**
     * @brief create a HoleSourceAgent at a specific site
     */
    HoleSourceAgent(World &world, int site, QObject *parent = 0);

    /**
     * @brief create a HoleSourceAgent at a specific Grid::CubeFace
     */
    HoleSourceAgent(World &world, Grid::CubeFace cubeFace, QObject *parent = 0);
protected:
    /**
     * @brief same as SourceAgent::validToInject(), but specialized for HoleAgents.
     */
    virtual bool validToInject(int site);

    /**
     * @brief same as FluxAgent::energyChange(), but specialized for HoleAgents.
     */
    virtual double energyChange(int site);

    /**
     * @brief same as SourceAgent::inject(), but specialized for HoleAgents.
     */
    virtual void inject(int site);
};

/**
 * @brief A class to inject Excitons
 */
class ExcitonSourceAgent : public SourceAgent
{
public:
    /**
     * @brief create an ExcitonSourceAgent
     */
    ExcitonSourceAgent(World &world, QObject *parent = 0);
protected:
    /**
     * @brief checks both grids if its ok to inject charges
     */
    virtual bool validToInject(int site);

    /**
     * @brief currently implemented as zero and not really used
     */
    virtual double energyChange(int site);

    /**
     * @brief uses the simple constant probability method
     */
    virtual bool shouldTransport(int site);

    /**
     * @brief choose a site to inject to
     *
     * reimplemented to chose a site at any grid site
     */
    virtual int chooseSite();

    /**
     * @brief similar to SourceAgent::inject(), but injects both a HoleAgent and an ElectronAgent
     */
    virtual void inject(int site);
};

}
#endif
