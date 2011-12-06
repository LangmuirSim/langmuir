/**
 * World - acts as a container for global information in the simulation
 */
#ifndef WORLD_H
#define WORLD_H
#define BOOST_DISABLE_ASSERTS

#include <QtCore>
#include <QtGui>
#include "boost/multi_array.hpp"

namespace Langmuir
{
  class Grid;
  class Agent;
  class Random;
  class Logger;
  class Potential;
  class DrainAgent;
  class SourceAgent;
  class ChargeAgent;
  class OpenClHelper;
  class Simulation;
  struct SimulationParameters;
  /**
    * @class World
    *
    * Stores the objects used in the simulation
    */
  class World
  {
   public:
    /**
      * Default Constructor.
      */
    World( SimulationParameters *par );

    /**
      * Default Destructor.
      */
   ~World();

    /**
      * @brief grid pointer
      */
    Grid * grid() { return m_grid; }

    /**
      * @brief potential pointer
      */
    Potential * potential() { return m_potential; }

    /**
      * @brief parameters pointer
      */
    SimulationParameters * parameters() { return m_parameters; }

    /**
      * @brief random number generator pointer
      */
    Random* randomNumberGenerator() { return m_rand; }

    /**
      * @brief random number generator pointer
      */
    SourceAgent* source() { return m_source; }

    /**
      * @brief random number generator pointer
      */
    DrainAgent* drain() { return m_drain; }

    /**
      * @brief random number generator pointer
      */
    Logger* logger() { return m_logger; }

    /**
      * @brief random number generator pointer
      */
    OpenClHelper* opencl() { return m_ocl; }

    /**
      * @brief simulation pointer
      */
    Simulation* simulation() { return m_simulation; }

    /**
      * @brief simulation pointer
      */
    void setSimulation( Simulation *sim ) { m_simulation =  sim; }

    /**
      * @brief charge agent pointer
      */
    QList<ChargeAgent *> * charges() { return &m_charges; }

    /**
      * @brief defect site id list pointer
      */
    QList<int> * defectSiteIDs() { return &m_defectSiteIDs; }

    /**
      * @brief trap site id list pointer
      */
    QList<int> * trapSiteIDs() { return &m_trapSiteIDs; }

    /**
      * @brief coupling matrix reference
      */
    boost::multi_array<double,2>& coupling() { return m_coupling; }

    /**
      * @brief interaction energy matrix reference
      */
    boost::multi_array<double,3>& interactionEnergies() { return m_interactionEnergies; }

   private:

    /**
      * @brief Holds site potentials and site ids
      */
    Grid *m_grid;

    /**
      * @brief Creates random doubles and ints
      */
    Random *m_rand;

    /**
      * @brief Accepts charge carriers
      */
    DrainAgent *m_drain;

    /**
      * @brief Injects charge carriers
      */
    SourceAgent *m_source;

    /**
      * @brief Calculates and sets potentials to be stored in the Grid
      */
    Potential *m_potential;

    /**
      * @brief All simple simulation parameters (ints,bools,doubles,strings) read by InputParser
      * @warning World does not allocate or destroy SimulationParameters
      */
    SimulationParameters *m_parameters;

    /**
      * @brief orchestrates simulation
      */
    Simulation *m_simulation;

    /**
      * @brief Writes various simulation information to files
      */
    Logger *m_logger;

    /**
      * @brief Handles initialization and usage of OpenCL for Coulomb calculations
      */
    OpenClHelper *m_ocl;

    /**
      * @brief A list of all the charge carriers for the system
      */
    QList<ChargeAgent *> m_charges;

    /**
      * @brief A list of all the defect site ids for the system
      */
    QList<int> m_defectSiteIDs;

    /**
      * @brief A list of all the trap site ids for the system
      */
    QList<int> m_trapSiteIDs;

    /**
      * @brief A list of coupling constants for transport between sites
      * The transport probability gets multiplied by these constants
      */
    boost::multi_array<double,2> m_coupling;

    /**
      * @brief Precomputed Coulomb interaction energies for CPU calculations
      */
    boost::multi_array<double,3> m_interactionEnergies;
 };
}
#endif // WORLD_H
