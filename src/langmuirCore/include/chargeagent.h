#ifndef CHARGEAGENT_H
#define CHARGEAGENT_H

#include "agent.h"

namespace LangmuirCore
{

class Grid;
struct SimulationParameters;

//! A class to represent moving charged particles
class ChargeAgent : public Agent
{
public:
    //! Construct charge
    /*!
     * \brief ChargeAgent
     * \param getType Agent type; must be Agent::Electron or Agent::Hole
     * \param world reference to world
     * \param grid reference to grid
     * \param site site id in grid
     * \param parent parent QObject
     */
    ChargeAgent(Agent::Type getType, World &world, Grid &grid, int site, QObject *parent=0);

    //! Destroy charge
    virtual ~ChargeAgent();

    //! Get the charge of the ChargeAgent
    int charge();

    //! Propose a random site to move to
    void chooseFuture();

    //! Decide what should happen, called after chooseFuture
    void decideFuture();

    //! Perform action, called after decideFuture
    void completeTick();

    //! True if decideFuture removed the charge from the grid
    bool removed();

    //! Number of steps ChargeAgent has existed
    int lifetime();

    //! Number of sites ChargeAgent has traversed
    int pathlength();

    //! Set the ChargeAgent OpenCL identifier
    /*!
      \see OpenClHelper
     */
    void setOpenCLID(int id);

    //! Get the ChargeAgent OpenCL identifier
    /*!
      \see OpenClHelper
     */
    int getOpenCLID();

    //! Perform coulombCPU() or coulombGPU()
    /*!
      depends upon SimulationParameters::useOpenCL and SimulationParameters::okCL
      \return ChargeAgent::m_de
     */
    double coulombInteraction();

    //! Calculate the Coulomb potential on the CPU
    /*!
      \note The result is stored in m_de
     */
    void coulombCPU();

    //! \b Retrieve the Coulomb potential from the GPU
    /*!
      \note The result is stored in m_de
      \warning this function assumes:
        - the openCL id set for the ChargeAgent is the correct one
        - the openCL kernel has been executed
     */
    void coulombGPU();

    //! compare results for CPU and GPU Coulomb (assumes kernel was called)
    void compareCoulomb();

    //! Get the grid this ChargeAgent exists in
    Grid& getGrid();

    //! Set the removed status of this ChargeAgent
    /*!
      \note Removed charges are not actually removed until completeTick() is called
     */
    void setRemoved(const bool &status = true);

    //! Return the opposite ChargeAgent type relative to this ChargeAgent
    /*!
     * \return Agent::Hole if this ChargeAgent is an Agent::Electron
     */
    virtual Agent::Type otherType() = 0;

    //! Return the opposite Grid relative to this ChargeAgent's Agent::Type
    /*!
     * \return World::holeGrid() if this chargeAgent is an Agent::Electron
     */
    virtual Grid& otherGrid() = 0;

protected:

    //! Calculate the exciton binding energy
    /*!
      \param site the site to check in other Grid
      \return
       - \f$+0.5\f$ eV if exciton
       - 0 otherwise
     */
    virtual double bindingPotential(int site)= 0;

    //! Charge of ChargeAgent (in units of e)
    int m_charge;

    //! Removed status of ChargeAgent
    bool m_removed;

    //! Number of steps ChargeAgent as been in existance
    int m_lifetime;

    //! Number of grid spaces ChargeAgent has moved
    int m_pathlength;

    //! The Grid the ChargeAgent lives in
    Grid &m_grid;

    //! The index of the Charge in the OpenCL vectors (see OpenClHelper)
    int m_openClID;

    //! The difference in Coulomb potential between ChargeAgent::m_site and ChargeAgent::m_fSite
    double m_de;
};

//! A class to represent moving negative charges
class ElectronAgent : public ChargeAgent
{
public:
    //! Construct ElectronAgent
    ElectronAgent(World &world, int site, QObject *parent=0);
protected:
    //! Calculate Exciton Binding Energy
    /*!
      \param site the site to check in other Grid
      \return
       - \f$+0.5\f$ eV if exciton
       - 0 otherwise
     */
    virtual double bindingPotential(int site);

    //! Return other Agent::Type
    /*!
      \return Agent::Hole
     */
    virtual Agent::Type otherType();

    //! Return other Grid
    /*!
      \return World::holeGrid
     */
    virtual Grid& otherGrid();
};

//! A class to represent moving positive charges
class HoleAgent : public ChargeAgent
{
public:
    //! Construct HoleAgent
    HoleAgent(World &world, int site, QObject *parent=0);
protected:
    //! Calculate Exciton Binding Energy
    /*!
      \param site the site to check in other Grid
      \return
       - \f$-0.5\f$ eV if exciton
       - 0 otherwise
     */
    virtual double bindingPotential(int site);

    //! Return other Agent::Type
    /*!
      \return Agent::Electron
     */
    virtual Agent::Type otherType();

    //! Return other Grid
    /*!
      \return World::electronGrid
     */
    virtual Grid& otherGrid();
};

}
#endif
