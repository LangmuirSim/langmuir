#ifndef OPENCLHELPER_H
#define OPENCLHELPER_H
#define __CL_ENABLE_EXCEPTIONS

#ifdef LANGMUIR_OPEN_CL
#include "cl.hpp"
#endif

#include <QObject>
#include <QVector>

namespace LangmuirCore
{

class World;
class ChargeAgent;

/**
 * @brief A class to run OpenCL calculations
 */
class OpenClHelper : public QObject
{
private:
    Q_OBJECT
    Q_DISABLE_COPY(OpenClHelper)

public:
    /**
     * @brief Create \b THE OpenClHelper; don't make more than one.
     * @param world reference to World Object
     * @param parent QObject this belongs to
     * @warning initializeOpenCL() must be called seperately
     */
    OpenClHelper(World &world, QObject *parent=0);

    /**
     * @brief Perform the tedious boilerplate code to initialize OpenCL
     */
    void initializeOpenCL(int gpuID = -1);

    /**
     * @brief Kernel1 calculates the coulomb potential at \b every site.
     *
     * This is extremely expensive on the CPU, it would take forever.  The GPU will
     * do it in a few seconds.  Luckily, the only reason to ever call this kernel
     * is if we want to save a snapshot of the coulomb potential - something that
     * is not needed during a normal simulation.
     */
    void launchCoulombKernel1();

    /**
     * @brief Kernel2 calculates the coulomb potential at current and future sites only
     */
    void launchCoulombKernel2();

    /**
     * @brief Kernel1 calculates the coulomb potential with erf at \b every site.
     */
    void launchGaussKernel1();

    /**
     * @brief Kernel2 calculates the coulomb potential with erf at current and future sites only
     */
    void launchGaussKernel2();

    /**
     * @brief Does exactly what it says (host means the memory on the CPU)
     * @param index position in host vectors
     * @param site serial site-id
     * @param charge charge of carrier
     */
    void copySiteAndChargeToHostVector(int index, int site, int charge = -1);

    /**
     * @brief Get the result stored in host memory (for current site)
     * @param index position in host vectors
     */
    double getOutputHost(int index) const;

    /**
     * @brief Get the result stored in host memory (for future site)
     * @param index position in host vectors
     *
     * There is a fixed offset in the host memory between the current and future site results
     */
    double getOutputHostFuture(int index) const;

    /**
     * @brief Compare GPU and CPU results
     */
    void compareHostAndDeviceForAllCarriers();

    /**
     * @brief Turn on/off OpenCL in a smart-way
     * @param on True if on
     * @return The on/off status
     *
     * For example, don't allow one to turn OpenCL on if OpenCL can't be used on this platform.
     */
    bool toggleOpenCL(bool on);

private:
    /**
     * @brief Reference to World object
     */
    World &m_world;

#ifdef LANGMUIR_OPEN_CL
    /**
     * @brief An OpenCL platform is the vendor (Intel, NVIDIA, AMD, etc)
     */
    cl::Platform m_platform;

    /**
     * @brief An OpenCL context is like a set of parameters for OpenCL (compare to an OpenGL context)
     */
    cl::Context m_context;

    /**
     * @brief The GPU
     */
    cl::Device m_device;

    /**
     * @brief A Queue to store a bunch of commands to OpenCL to execute (in our case, the queue is in serial mode)
     */
    cl::CommandQueue m_queue;

    /**
     * @brief Coulomb Kernel 1
     */
    cl::Kernel m_coulomb1K;

    /**
     * @brief Gaussian Kernel 1
     */
    cl::Kernel m_guass1K;

    /**
     * @brief Coulomb Kernel 2
     */
    cl::Kernel m_coulomb2K;

    /**
     * @brief Gaussian Kernel 2
     */
    cl::Kernel m_guass2K;

    /**
     * @brief Memory on the host (CPU) to store site-ids
     *
     * This is extremely annoying and ineffcient, but site-ids have to be copied here first,
     * before they are copied to the GPU.  The same goes for the charges, and the output.
     * OpenCL requires that data to be copied to/from the device (GPU) must be continuous
     * in memory on the host (CPU).  So, that means its impossible to copy data directly from
     * a set of ChargeAgents.  Theres got to be a way around this, copying things \b twice
     * is slow.  You would think that the ChargeAgent's data is continuous in memory, but
     * offset by some amount between instances, for a particular member.
     */
    QVector<int> m_sHost;

    /**
     * @brief Memory on the host (CPU) to store charge values
     */
    QVector<int> m_qHost;

    /**
     * @brief Memory on the host (CPU) to store output values
     */
    QVector<double> m_oHost;

    /**
     * @brief Memory on the device (GPU) to store site-ids. It is in the global memory of the device
     */
    cl::Buffer m_sDevice;

    /**
     * @brief Memory on the device (GPU) to store charge values. It is in the global memory of the device
     */
    cl::Buffer m_qDevice;

    /**
     * @brief Memory on the device (GPU) to store output values. It is in the global memory of the device
     */
    cl::Buffer m_oDevice;

    /**
     * @brief Offset between current and future output values in m_oDevice or m_oHost.
     *
     * For example, if the output for current sites starts at index 0 and runs up to \b offset.
     * Then, the output for future sites starts at \b offset and runs up to \b 2 \b offset.
     */
    int m_offset;
#endif // LANGMUIR_OPEN_CL

};
}

#endif // OPENCLHELPER_H
