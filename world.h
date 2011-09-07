/**
 * World - acts as a container for global information in the simulation
 */
#ifndef WORLD_H
#define WORLD_H
#define __CL_ENABLE_EXCEPTIONS

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
    std::vector<double> values;

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
      * @brief Turn OpenCL error code into string
      */
    QString clErrorString (cl_int error);

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

    cl_int                        m_error;       // OpenCL error code
    std::vector<cl::Platform>     m_platforms;   // OpenCL platforms
    std::vector<cl::Context>      m_contexts;    // OpenCL contexts
    std::vector<cl::Device>       m_devices;     // OpenCL devices
    std::vector<cl::CommandQueue> m_queues;      // OpenCL queues
    std::vector<cl::Program>      m_programs;    // OpenCL programs
    std::vector<cl::Kernel>       m_kernels;     // OpenCL kernels
    std::vector<int>              m_iHost;       // Host site ids (initial)
    std::vector<int>              m_fHost;       // Host site ids (final)
    std::vector<double>           m_oHost;       // Host output vector
    cl::Buffer                    m_iDevice;     // Device site ids (initial)
    cl::Buffer                    m_fDevice;     // Device site ids (final)
    cl::Buffer                    m_dDevice;     // Device site ids (for defects)
    cl::Buffer                    m_oDevice;     // Device output vector
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

 inline QString World::clErrorString (cl_int error)
 {
   if (error == CL_SUCCESS)
     return "CL_SUCCESS";
   else if (error == CL_DEVICE_NOT_FOUND)
     return "CL_DEVICE_NOT_FOUND";
   else if (error == CL_DEVICE_NOT_AVAILABLE)
     return "CL_DEVICE_NOT_AVAILABLE";
   else if (error == CL_COMPILER_NOT_AVAILABLE)
     return "CL_COMPILER_NOT_AVAILABLE";
   else if (error == CL_MEM_OBJECT_ALLOCATION_FAILURE)
     return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
   else if (error == CL_OUT_OF_RESOURCES)
     return "CL_OUT_OF_RESOURCES";
   else if (error == CL_OUT_OF_HOST_MEMORY)
     return "CL_OUT_OF_HOST_MEMORY";
   else if (error == CL_PROFILING_INFO_NOT_AVAILABLE)
     return "CL_PROFILING_INFO_NOT_AVAILABLE";
   else if (error == CL_MEM_COPY_OVERLAP)
     return "CL_MEM_COPY_OVERLAP";
   else if (error == CL_IMAGE_FORMAT_MISMATCH)
     return "CL_IMAGE_FORMAT_MISMATCH";
   else if (error == CL_IMAGE_FORMAT_NOT_SUPPORTED)
     return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
   else if (error == CL_BUILD_PROGRAM_FAILURE)
     return "CL_BUILD_PROGRAM_FAILURE";
   else if (error == CL_MAP_FAILURE)
     return "CL_MAP_FAILURE";
   //else if (error == CL_MISALIGNED_SUB_BUFFER_OFFSET)
   //  return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
   //else if (error == CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST)
   //  return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
   else if (error == CL_INVALID_VALUE)
     return "CL_INVALID_VALUE";
   else if (error == CL_INVALID_DEVICE_TYPE)
     return "CL_INVALID_DEVICE_TYPE";
   else if (error == CL_INVALID_PLATFORM)
     return "CL_INVALID_PLATFORM";
   else if (error == CL_INVALID_DEVICE)
     return "CL_INVALID_DEVICE";
   else if (error == CL_INVALID_CONTEXT)
     return "CL_INVALID_CONTEXT";
   else if (error == CL_INVALID_QUEUE_PROPERTIES)
     return "CL_INVALID_QUEUE_PROPERTIES";
   else if (error == CL_INVALID_COMMAND_QUEUE)
     return "CL_INVALID_COMMAND_QUEUE";
   else if (error == CL_INVALID_HOST_PTR)
     return "CL_INVALID_HOST_PTR";
   else if (error == CL_INVALID_MEM_OBJECT)
     return "CL_INVALID_MEM_OBJECT";
   else if (error == CL_INVALID_IMAGE_FORMAT_DESCRIPTOR)
     return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
   else if (error == CL_INVALID_IMAGE_SIZE)
     return "CL_INVALID_IMAGE_SIZE";
   else if (error == CL_INVALID_SAMPLER)
     return "CL_INVALID_SAMPLER";
   else if (error == CL_INVALID_BINARY)
     return "CL_INVALID_BINARY";
   else if (error == CL_INVALID_BUILD_OPTIONS)
     return "CL_INVALID_BUILD_OPTIONS";
   else if (error == CL_INVALID_PROGRAM)
     return "CL_INVALID_PROGRAM";
   else if (error == CL_INVALID_PROGRAM_EXECUTABLE)
     return "CL_INVALID_PROGRAM_EXECUTABLE";
   else if (error == CL_INVALID_KERNEL_NAME)
     return "CL_INVALID_KERNEL_NAME";
   else if (error == CL_INVALID_KERNEL_DEFINITION)
     return "CL_INVALID_KERNEL_DEFINITION";
   else if (error == CL_INVALID_KERNEL)
     return "CL_INVALID_KERNEL";
   else if (error == CL_INVALID_ARG_INDEX)
     return "CL_INVALID_ARG_INDEX";
   else if (error == CL_INVALID_ARG_VALUE)
     return "CL_INVALID_ARG_VALUE";
   else if (error == CL_INVALID_ARG_SIZE)
     return "CL_INVALID_ARG_SIZE";
   else if (error == CL_INVALID_KERNEL_ARGS)
     return "CL_INVALID_KERNEL_ARGS";
   else if (error == CL_INVALID_WORK_DIMENSION)
     return "CL_INVALID_WORK_DIMENSION";
   else if (error == CL_INVALID_WORK_GROUP_SIZE)
     return "CL_INVALID_WORK_GROUP_SIZE";
   else if (error == CL_INVALID_WORK_ITEM_SIZE)
     return "CL_INVALID_WORK_ITEM_SIZE";
   else if (error == CL_INVALID_GLOBAL_OFFSET)
     return "CL_INVALID_GLOBAL_OFFSET";
   else if (error == CL_INVALID_EVENT_WAIT_LIST)
     return "CL_INVALID_EVENT_WAIT_LIST";
   else if (error == CL_INVALID_EVENT)
     return "CL_INVALID_EVENT";
   else if (error == CL_INVALID_OPERATION)
     return "CL_INVALID_OPERATION";
   else if (error == CL_INVALID_GL_OBJECT)
     return "CL_INVALID_GL_OBJECT";
   else if (error == CL_INVALID_BUFFER_SIZE)
     return "CL_INVALID_BUFFER_SIZE";
   else if (error == CL_INVALID_MIP_LEVEL)
     return "CL_INVALID_MIP_LEVEL";
   else if (error == CL_INVALID_GLOBAL_WORK_SIZE)
     return "CL_INVALID_GLOBAL_OFFSET";
   else if (error == CL_INVALID_EVENT_WAIT_LIST)
     return "CL_INVALID_EVENT_WAIT_LIST";
   else if (error == CL_INVALID_EVENT)
     return "CL_INVALID_EVENT";
   else if (error == CL_INVALID_OPERATION)
     return "CL_INVALID_OPERATION";
   else if (error == CL_INVALID_GL_OBJECT)
     return "CL_INVALID_GL_OBJECT";
   else if (error == CL_INVALID_BUFFER_SIZE)
     return "CL_INVALID_BUFFER_SIZE";
   else if (error == CL_INVALID_MIP_LEVEL)
     return "CL_INVALID_MIP_LEVEL";
   else if (error == CL_INVALID_GLOBAL_WORK_SIZE)
     return "CL_INVALID_GLOBAL_WORK_SIZE";
   //else if (error == CL_INVALID_PROPERTY)
   //  return "CL_INVALID_PROPERTY";
   else
     return "CL_UNKNOWN_ERROR";
 }

}

#endif // WORLD_H
