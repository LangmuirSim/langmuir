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
  class Simulation;
  class ChargeAgent;
  class SourceAgent;
  class OpenClHelper;
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
    Grid * electronGrid() { return m_electronGrid; }

    /**
      * @brief grid pointer
      */
    Grid * holeGrid() { return m_holeGrid; }

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
      * @brief source pointer
      */
    SourceAgent* holeSourceL() { return m_holeSourceL; }

    /**
      * @brief source pointer
      */
    SourceAgent* holeSourceR() { return m_holeSourceR; }

    /**
      * @brief source pointer
      */
    SourceAgent* electronSourceL() { return m_electronSourceL; }

    /**
      * @brief source pointer
      */
    SourceAgent* electronSourceR() { return m_electronSourceR; }

    /**
      * @brief source pointer
      */
    SourceAgent* excitonSource() { return m_excitonSource; }

    /**
      * @brief drain pointer
      */
    DrainAgent* holeDrainL() { return m_holeDrainL; }

    /**
      * @brief drain pointer
      */
    DrainAgent* holeDrainR() { return m_holeDrainR; }

    /**
      * @brief drain pointer
      */
    DrainAgent* electronDrainL() { return m_electronDrainL; }

    /**
      * @brief drain pointer
      */
    DrainAgent* electronDrainR() { return m_electronDrainR; }

    /**
      * @brief logger pointer
      */
    Logger* logger() { return m_logger; }

    /**
      * @brief openCL pointer
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
    QList<ChargeAgent *> * electrons() { return &m_electrons; }

    /**
      * @brief charge agent pointer
      */
    QList<ChargeAgent *> * holes() { return &m_holes; }

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
    Grid *m_electronGrid;

    /**
      * @brief Holds site potentials and site ids
      */
    Grid *m_holeGrid;

    /**
      * @brief Creates random doubles and ints
      */
    Random *m_rand;

    /**
      * @brief Accepts holes at x = grid.width
      */
    DrainAgent *m_holeDrainR;

    /**
      * @brief Accepts holes at x = 0
      */
    DrainAgent *m_holeDrainL;

    /**
      * @brief Accepts electrons at x = grid.width
      */
    DrainAgent *m_electronDrainR;

    /**
      * @brief Accepts electrons at x = 0
      */
    DrainAgent *m_electronDrainL;

    /**
      * @brief Injects electrons at x = 0
      */
    SourceAgent *m_electronSourceL;

    /**
      * @brief Injects electrons at x = grid.width
      */
    SourceAgent *m_electronSourceR;

    /**
      * @brief Injects holes at x = 0
      */
    SourceAgent *m_holeSourceL;

    /**
      * @brief Injects holes at x = grid.width
      */
    SourceAgent *m_holeSourceR;

    /**
      * @brief Injects excitons at random sites
      */
    SourceAgent *m_excitonSource;

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
    QList<ChargeAgent *> m_electrons;

    /**
      * @brief A list of all the charge carriers for the system
      */
    QList<ChargeAgent *> m_holes;

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
      * Somtimes the transport probabilty is the boltzmann factor,
      * other times it is 1; when it is 1 the coupling constant is
      * essentially the probability
      */
    boost::multi_array<double,2> m_coupling;

    /**
      * @brief Precomputed Coulomb interaction energies for CPU calculations
      */
    boost::multi_array<double,3> m_interactionEnergies;
 };
}
#endif // WORLD_H
