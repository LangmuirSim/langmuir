#ifndef OPENCLHELPER_H
#define OPENCLHELPER_H
#define __CL_ENABLE_EXCEPTIONS
#define __NO_STD_VECTOR

#include "cl.hpp"
#include <QObject>
#include <QVector>

namespace Langmuir
{

class World;
class ChargeAgent;

class OpenClHelper : public QObject
{
private:
    Q_OBJECT
    Q_DISABLE_COPY(OpenClHelper)

public:
    OpenClHelper(World &world, QObject *parent=0);
    void initializeOpenCL();
    void launchCoulombKernel1();
    void launchCoulombKernel2();
    void copySiteAndChargeToHostVector(int index, int site, int charge = -1);
    double getOutputHost(int index) const;
    double getOutputHostFuture(int index ) const;
    void compareHostAndDeviceForAllCarriers();
    void compareHostAndDeviceAtSite(int i);
    void compareHostAndDeviceForCarrier(int i, QList< ChargeAgent * > &charges);
    bool toggleOpenCL(bool on);

private:
    World                       &m_world;
#ifdef LANGMUIR_OPEN_CL
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
#endif // LANGMUIR_OPEN_CL
};
}

#endif // OPENCLHELPER_H
