#ifndef OPENCLHELPER_H
#define OPENCLHELPER_H
#define __CL_ENABLE_EXCEPTIONS
#define __NO_STD_VECTOR

#include <QtCore>
#include "cl.hpp"

namespace Langmuir
{
class World;
class ChargeAgent;
/**
  *  @class OpenClHelper
  *  @brief For handling all OpenCl related tasks
  */
class OpenClHelper
{
public:
    OpenClHelper( World * world );

    /**
      * @brief Set up OpenCL enviroment
      */
    void initializeOpenCL();

    /**
      * @brief tell the GPU to perform coulomb calculations over all locations
      */
    void launchCoulombKernel1();

    /**
      * @brief tell the GPU to perform coulomb calculations over select locations
      */
    void launchCoulombKernel2();

    /**
      * @brief copy a charge carrier's current site ID to the m_iHost vector
      */
    inline void copySiteAndChargeToHostVector( int index, int site, int charge = -1 ) { m_sHost[index] = site; m_qHost[index] = charge; }

    /**
      * @brief read a value in m_oDevice - which should be the result of a coulomb calculation.
      */
    inline const double& getOutputHost(int index) const { return m_oHost[index]; }

    /**
      * @brief read a value in m_oDevice - which should be the result of a coulomb calculation.
      */
    inline const double& getOutputHostFuture(int index) const { return m_oHost[index+m_offset]; }

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
    void compareHostAndDeviceForCarrier(int i,QList< ChargeAgent * > &charges);

    /**
     * @brief change parameters
     */
    bool toggleOpenCL(bool on);

private:
    World                       *m_world;
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
    int                          m_offset;              // Offset between current and future energies in m_oHost;
};
}

#endif // OPENCLHELPER_H
