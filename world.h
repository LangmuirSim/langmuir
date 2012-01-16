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
class World : public QObject
{
Q_OBJECT public:
    /**
      * Default Constructor.
      */
    World(SimulationParameters *par, QObject *parent = 0);

    /**
      * Default Destructor.
      */
    ~World();

    /**
      * @brief grid pointer
      */
    Grid * electronGrid(){ return m_electronGrid; }

    /**
      * @brief grid pointer
      */
    Grid * holeGrid(){ return m_holeGrid; }

    /**
      * @brief potential pointer
      */
    Potential * potential(){ return m_potential; }

    /**
      * @brief parameters pointer
      */
    SimulationParameters * parameters(){ return m_parameters; }

    /**
      * @brief random number generator pointer
      */
    Random* randomNumberGenerator(){ return m_rand; }

    /**
      * @brief logger pointer
      */
    Logger* logger(){ return m_logger; }

    /**
      * @brief openCL pointer
      */
    OpenClHelper* opencl(){ return m_ocl; }

    /**
      * @brief source agent pointer
      */
    QList<SourceAgent *> * sources(){ return &m_sources; }

    /**
      * @brief drain agent pointer
      */
    QList<DrainAgent *> * drains(){ return &m_drains; }

    /**
      * @brief charge agent pointer
      */
    QList<ChargeAgent *> * electrons(){ return &m_electrons; }

    /**
      * @brief charge agent pointer
      */
    QList<ChargeAgent *> * holes(){ return &m_holes; }

    /**
      * @brief defect site id list pointer
      */
    QList<int> * defectSiteIDs(){ return &m_defectSiteIDs; }

    /**
      * @brief trap site id list pointer
      */
    QList<int> * trapSiteIDs(){ return &m_trapSiteIDs; }

    /**
      * @brief interaction energy matrix reference
      */
    boost::multi_array<double, 3>& interactionEnergies(){ return m_interactionEnergies; }

private:

    /**
      * @brief A list of all the sources for the system
      */
    QList<SourceAgent *> m_sources;

    /**
      * @brief A list of all the drains for the system
      */
    QList<DrainAgent *> m_drains;

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
      * @brief Calculates and sets potentials to be stored in the Grid
      */
    Potential *m_potential;

    /**
      * @brief All simple simulation parameters(ints, bools, doubles, strings)read by InputParser
      * @warning World does not allocate or destroy SimulationParameters
      */
    SimulationParameters *m_parameters;

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
      * @brief Precomputed Coulomb interaction energies for CPU calculations
      */
    boost::multi_array<double, 3> m_interactionEnergies;
};
}
#endif // WORLD_H
