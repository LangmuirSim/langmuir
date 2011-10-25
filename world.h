/**
 * World - acts as a container for global information in the simulation
 */
#ifndef WORLD_H
#define WORLD_H
#define BOOST_DISABLE_ASSERTS
#define __CL_ENABLE_EXCEPTIONS
#define __NO_STD_VECTOR

#include "cl.hpp"
#include <QtCore>
#include <QtGui>
#include "boost/multi_array.hpp"

namespace Langmuir
{
  class Grid;
  class Random;
  class ChargeAgent;
  struct SimulationParameters;

  /**
    * @class World
    *
    * Holds information for simulation.
    */
  class World
  {
   public:
    /**
      * Default Constructor.
      */
    World( int seed = -1 );

    /**
      * Default Destructor.
      */
   ~World();

    /**
      * @brief Set up OpenCL enviroment
      */
    void initializeOpenCL();

    /**
      * @brief member access.
      *
      * Get the address of the grid object.
      */
    Grid * grid() const { return m_grid; }

    /**
      * @brief member access.
      *
      * Set the address of the grid object.
      */
    void setGrid(Grid *grid) { m_grid = grid; }

    /**
      * @brief member access.
      *
      * Get the address of the parameters object.
      */
    SimulationParameters * parameters() const { return m_parameters; }

    /**
      * @brief member access.
      *
      * Set the address of the parameters object.  Initialize stat file if necessary.
      */
    void setParameters(SimulationParameters *parameters);

    /**
      * @brief member access.
      *
      * Get the address of the Random Number Generator object.
      */
    Random* randomNumberGenerator() { return m_rand; }

    /**
      * @brief member access.
      *
      * Get the address of a list of charge agents.
      */
    QList<ChargeAgent *> * charges();

    /**
      * @brief member access.
      *
      * Get the address of a list of charged defects.
      */
    QList<int> * defectSiteIDs();

    /**
      * @brief member access.
      *
      * Get the address of a list of charged traps.
      */
    QList<int> * trapSiteIDs();

    /**
      * @brief member access.
      *
      * Get the address of a list of coupling constants.
      */
    boost::multi_array<double,2>& coupling();

    /**
      * @brief member access.
      *
      * Get a reference to a distance matrix. 
      */
    boost::multi_array<double,3>& interactionEnergies();

    /**
      * @brief tell the GPU to perform coulomb calculations over all locations
      */
    void launchCoulombKernel1();

    /**
      * @brief tell the GPU to perform coulomb calculations over select locations
      */
    void launchCoulombKernel2();

    /**
      * @brief save carrier ids to a file
      */
    void saveCarrierIDsToFile( QString name );

    /**
      * @brief save trap ids to a file
      */
    void saveTrapIDsToFile( QString name );

    /**
      * @brief save defect ids to a file
      */
    void saveDefectIDsToFile( QString name );

    /**
      * @brief save field energy to a file
      */
    void saveFieldEnergyToFile( QString name );

    /**
      * @brief save trap energy to a file
      */
    void saveTrapEnergyToFile( QString name );

    /**
      * @brief save coulomb energy to a file
      */
    void saveCoulombEnergyToFile( QString name );

    /**
      * @brief copy a charge carrier's current site ID to the m_iHost vector
      *
      * had to do this because to copy to the GPU we need a closely packed array of values.
      * The data *needed* from each Agent object isn't closely packed.  Wish there was a
      * way around this - its an extra copying step.
      */
    inline void copySiteAndChargeToHostVector( int index, int site, int charge = -1 ) { m_sHost[index] = site; m_qHost[index] = charge; }

    /**
      * @brief read a value in m_oDevice - which should be the result of a coulomb calculation.
      */
    inline const double& getOutputHost(int index) const { return m_oHost[index]; }

    /**
      * @brief compare GPU answer (delta energy) to CPU answer for all charges
      */
    void compareHostAndDeviceForAllCarriers();

    /**
      * @brief compare GPU answer to CPU answer (at single point)
      */
    void compareHostAndDeviceAtSite(int i);

    /**
      * @brief compare GPU answer (delta energy) to CPU answer for a single charge
      */
    void compareHostAndDeviceForCarrier(int i);

    /**
     * @brief change parameters
     *
     * toggle openCL if valid
     */
    bool toggleOpenCL(bool on);

    /**
      * @brief write message to stat file
      */
    void statMessage(QString message);

    /**
      * @brief flush the stat file
      */
    void statFlush();

   private:

    Grid                        *m_grid;                // The grid in use in the world
    Random                      *m_rand;                // Random number generator
    SimulationParameters        *m_parameters;          // Simulation Parameters
    QList<ChargeAgent *>         m_charges;             // Charge carriers in the system
    QList<int>                   m_defectSiteIDs;       // Site ids of defects in the system
    QList<int>                   m_trapSiteIDs;         // Site ids of traps in the system
    boost::multi_array<double,2> m_coupling;            // Matrix of coupling constants
    boost::multi_array<double,3> m_interactionEnergies; // Interaction energies
    QFile                       *m_statFile;            // Place to write lifetime and pathlength information
    QTextStream                 *m_statStream;          // Text stream for stat file
    cl::Context                  m_context;             // OpenCl context
    cl::CommandQueue             m_queue;               // OpenCl queue
    cl::Kernel                   m_coulombK1;           // Kernel for calculating Coulomb Energy everywhere
    cl::Kernel                   m_coulombK2;           // Kernel for calculating Coulomb Energy at select places
    QVector<int>                 m_sHost;               // Host site ids
    QVector<int>                 m_qHost;               // Host charges
    QVector<double>              m_oHost;               // Host output vector
    cl::Buffer                   m_sDevice;             // Device site ids
    cl::Buffer                   m_qDevice;             // Device charges
    cl::Buffer                   m_oDevice;             // Device output vector
 };

 inline QList<ChargeAgent *> * World::charges()
 {
  return &m_charges;
 }

 inline QList<int> * World::defectSiteIDs()
 {
  return &m_defectSiteIDs;
 }

 inline QList<int> * World::trapSiteIDs()
 {
  return &m_trapSiteIDs;
 }

 inline boost::multi_array<double,2>& World::coupling()
 {
  return m_coupling;
 }

 inline boost::multi_array<double,3>& World::interactionEnergies()
 {
  return m_interactionEnergies;
 }

}
#endif // WORLD_H
