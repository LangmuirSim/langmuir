/**
 * World - acts as a container for global information in the simulation
 */
#ifndef WORLD_H
#define WORLD_H
#define __CL_ENABLE_EXCEPTIONS
#define __NO_STD_VECTOR

#include "cl.hpp"
#include <QtCore>
#include <vector>
#include <Eigen/Core>

namespace Langmuir
{
  class Grid;
  class Rand;
  class ChargeAgent;
  struct SimulationParameters;

  /**
    * @class Triple Index Array
    * @brief Three dimensional matrix
    *
    * For storing doubles to be accessed by col, row, and lay indices.
    */
  class TripleIndexArray
  {
   public:
    /**
      * @brief Default constructor.
      *
      * Sets everything to zero.
      */
    TripleIndexArray ();

    /**
      * @brief Alternate constructor.
      *
      * Sets columns, rows, layers, area, and width to indicated values. 
      * Resizes the internal vector to hold the appropriate number of values.
      */
    TripleIndexArray (unsigned int col, unsigned int row, unsigned int lay);

    /** 
      * @brief operator overload
      *
      * Access a double at col, row, lay position.
      */
    double& operator() (unsigned int col, unsigned int row, unsigned int lay);

    /** 
      * @brief operator overload 
      *
      * Access a double at col, row, lay position.
      */
    double operator() (unsigned int col, unsigned int row, unsigned int lay) const;

    /**
      * @brief Clear values and reallocate space.
      *
      * Sets columns, rows, layers, area, and width to indicated values. 
      * Resizes the internal vector to hold the appropriate number of values.
      */
    void resize (unsigned int col, unsigned int row, unsigned int lay);

    /**
      * @brief member access.
      *
      * Get the number of rows. 
      */
    inline unsigned int rows() const { return m_row; }

    /**
      * @brief member access.
      *
      * Get the number of columns.
      */
    inline unsigned int cols() const { return m_col; }

    /**
      * @brief member access.
      *
      * Get the number of layerss.
      */
    inline unsigned int lays() const { return m_lay; }

    /**
      * @brief member access.
      *
      * Get the number of values.
      */
    inline unsigned int size() const { return values.size(); }

   private:

    /**
      * @brief member.
      *
      * Array of doubles.
      */
    QVector<double> values;

    /**
      * @brief member.
      *
      * The number of columns.
      */
    unsigned int m_width;

    /**
      * @brief member.
      *
      * The number of columns times the number of rows.
      * The total number of items in a layer.
      */
    unsigned int m_area;

    /**
      * @brief member.
      *
      * The number of columns.
      */
    unsigned int m_col;

    /**
      * @brief member.
      *
      * The number of rows.
      */
    unsigned int m_row;

    /**
      * @brief member.
      *
      * The number of layers.
      */
    unsigned int m_lay;
  };

  const unsigned int errorValue = -1;

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
      * @brief random number generation.
      *
      * Get a random number between 0 and 1.
      */
    double random();

    /**
      * @brief random number generation.
      *
      * Get a random number with average and standard deviation of gaussian.
      * @param average average of gaussian
      * @param stdev standard deviation of gaussian
      */
    double random( double average, double stdev );

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
    QList<unsigned int> * defectSiteIDs();

    /**
      * @brief member access.
      *
      * Get the address of a list of charged traps.
      */
    QList<unsigned int> * trapSiteIDs();

    /**
      * @brief member access.
      *
      * Get the address of a list of coupling constants.
      */
    Eigen::MatrixXd * coupling();

    /**
      * @brief member access.
      *
      * Get a reference to a distance matrix. 
      */
    TripleIndexArray& interactionEnergies();

    /**
      * @brief copy n = m_charges.size() doubles from GPU to CPU
      *
      * these doubles are put inside m_oDevice; they should contain the results of coulomb calculations.
      */
    void copyDeviceToHost( );

    /**
      * @brief copy n = m_charges.size() * 2 ints from CPU to GPU
      *
      * the ints are the current and future(proposed) site IDs of charge carriers
      */
    void copyHostToDevice( );

    /**
      * @brief tell the GPU to perform coulomb calculations
      */
    void launchKernel( );

    /**
      * @brief wait for the GPU to be done
      */
    void clFinish( );

    /**
      * @brief copy a charge carrier's current site ID to the m_iHost vector
      *
      * had to do this because to copy to the GPU we need a closely packed array of values.
      * The data *needed* from each Agent object isn't closely packed.  Wish there was a
      * way around this - its an extra copying step.
      */
    inline void setISiteHost( int index, int value ) { m_iHost[index] = value; }

    /**
      * @brief copy a charge carrier's future site ID to the m_iHost vector
      */
    inline void setFSiteHost( int index, int value ) { m_fHost[index] = value; }

    /**
      * @brief read a value in m_oDevice - which should be the result of a coulomb calculation.
      */
    inline const double& getOutputHost( int index ) const { return m_oHost[index]; }

    /**
      * @brief if or not OpenCL could be set up correctly and used
      */
    const bool canUseOpenCL() const;

    /**
      * @brief write message to stat file
      */
    void statMessage( QString message );

    /**
      * @brief flush the stat file
      */
    void statFlush( );

   private:

    Grid                 *m_grid;                // The grid in use in the world
    Rand                 *m_rand;                // Random number generator
    SimulationParameters *m_parameters;          // Simulation Parameters
    QList<ChargeAgent *>  m_charges;             // Charge carriers in the system
    QList<unsigned int>   m_defectSiteIDs;       // Site ids of defects in the system
    QList<unsigned int>   m_trapSiteIDs;         // Site ids of traps in the system
    Eigen::MatrixXd       m_coupling;            // Enumerates coupling constants between different sites
    TripleIndexArray      m_interactionEnergies; // Interaction energies
    QFile                *m_statFile;             // Place to write lifetime and pathlength information
    QTextStream          *m_statStream;           // Text stream for stat file

    cl_int                       m_error;       // OpenCL error code
    cl::vector<cl::Platform>     m_platforms;   // OpenCL platforms
    cl::vector<cl::Context>      m_contexts;    // OpenCL contexts
    cl::vector<cl::Device>       m_devices;     // OpenCL devices
    cl::vector<cl::CommandQueue> m_queues;      // OpenCL queues
    cl::vector<cl::Program>      m_programs;    // OpenCL programs
    cl::vector<cl::Kernel>       m_kernels;     // OpenCL kernels
    QVector<int>                 m_iHost;       // Host site ids (initial)
    QVector<int>                 m_fHost;       // Host site ids (final)
    QVector<double>              m_oHost;       // Host output vector
    cl::Buffer                   m_iDevice;     // Device site ids (initial)
    cl::Buffer                   m_fDevice;     // Device site ids (final)
    cl::Buffer                   m_dDevice;     // Device site ids (for defects)
    cl::Buffer                   m_oDevice;     // Device output vector
 };

 inline QList<ChargeAgent *> * World::charges()
 {
  return &m_charges;
 }

 inline QList<unsigned int> * World::defectSiteIDs()
 {
  return &m_defectSiteIDs;
 }

 inline QList<unsigned int> * World::trapSiteIDs()
 {
  return &m_trapSiteIDs;
 }

 inline Eigen::MatrixXd * World::coupling()
 {
  return &m_coupling;
 }

 inline TripleIndexArray& World::interactionEnergies()
 {
  return m_interactionEnergies;
 }

}

#endif // WORLD_H
