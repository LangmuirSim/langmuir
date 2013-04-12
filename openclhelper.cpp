#include <QTextStream>
#include "pbsgpuparser.h"
#include "openclhelper.h"
#include "chargeagent.h"
#include "parameters.h"
#include "cubicgrid.h"
#include "potential.h"
#include "world.h"

namespace Langmuir
{

OpenClHelper::OpenClHelper(World &world, QObject *parent):
    QObject(parent), m_world(world)
{
}

void OpenClHelper::initializeOpenCL()
{
    //can't use openCL yet
    m_world.parameters().okCL = false;

#ifdef LANGMUIR_OPEN_CL
    try
    {   
        //obtain platforms
        std::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);
        m_platform = platforms.at(0);

        //obtain all devices
        std::vector<cl::Device> all_devices;
        m_platform.getDevices(CL_DEVICE_TYPE_GPU, &all_devices);

        //choose a single device
        PBSGPUParser parser;
        std::vector<cl::Device> devices;
        devices.push_back(all_devices.at(parser.gpu()));
        m_device = devices.at(0);

        //obtain context
        cl_context_properties contextProperties[3] = {
            CL_CONTEXT_PLATFORM,(cl_context_properties)platforms[0](), 0
        };
        m_context = cl::Context(devices, contextProperties);

        //obtain command queue
        m_queue = cl::CommandQueue(m_context, m_device);
        m_queue.finish();

        //obtain kernel source
        QFile file(":/resources/kernel.cl");
        if(!file.open(QIODevice::ReadOnly))
        {
            qDebug("message: error opening file: :/resources.kernel.cl");
            throw cl::Error(-1, "OpenCL Error!");
        }
        QByteArray lines = file.readAll();
        file.close();

        //create program
        cl::Program::Sources source(1, std::make_pair(lines, lines.size()));
        cl::Program program(m_context, source);
        program.build(devices);

        //create kernels
        m_coulomb1K = cl::Kernel(program, "coulomb1");
        m_coulomb2K = cl::Kernel(program, "coulomb2");
        m_guass1K = cl::Kernel(program, "gauss1");
        m_guass2K = cl::Kernel(program, "gauss2");

        //initialize Host Memory
        m_sHost.clear();
        m_qHost.clear();
        m_oHost.clear();
        m_sHost.resize(m_world.electronGrid().volume());
        m_qHost.resize(m_world.electronGrid().volume());
        m_oHost.resize(m_world.electronGrid().volume());

        //calculate memory sizes
        size_t sSize = m_sHost.size() * sizeof(int);
        size_t qSize = m_qHost.size() * sizeof(int);
        size_t oSize = m_oHost.size() * sizeof(double);

        //initialize Device Memory
        m_sDevice = cl::Buffer(m_context, CL_MEM_READ_ONLY , sSize);
        m_qDevice = cl::Buffer(m_context, CL_MEM_READ_ONLY , qSize);
        m_oDevice = cl::Buffer(m_context, CL_MEM_READ_WRITE, oSize);

        //upload initial data
        m_queue.enqueueWriteBuffer(m_sDevice, CL_TRUE, 0, sSize, &m_sHost[0]);
        m_queue.enqueueWriteBuffer(m_sDevice, CL_TRUE, 0, qSize, &m_qHost[0]);
        m_queue.enqueueWriteBuffer(m_oDevice, CL_TRUE, 0, oSize, &m_oHost[0]);

        //preset kernel arguments that dont change
        int cutoff2 = m_world.parameters().electrostaticCutoff *
            m_world.parameters().electrostaticCutoff;
        double erffactor = 0.0;
        if (m_world.parameters().coulombGaussianSigma > 0)
        {
            erffactor = sqrt(2.0) * m_world.parameters().coulombGaussianSigma;
            erffactor = 1.0 / erffactor;
        }

        // coulomb kernel 1
        m_coulomb1K.setArg(0, m_oDevice);
        m_coulomb1K.setArg(1, m_sDevice);
        m_coulomb1K.setArg(2, m_qDevice);
        m_coulomb1K.setArg(4, cutoff2);
        m_coulomb1K.setArg(5, m_world.parameters().electrostaticPrefactor);

        // gauss kernel 1
        m_guass1K.setArg(0, m_oDevice);
        m_guass1K.setArg(1, m_sDevice);
        m_guass1K.setArg(2, m_qDevice);
        m_guass1K.setArg(4, cutoff2);
        m_guass1K.setArg(5, m_world.parameters().electrostaticPrefactor);
        m_guass1K.setArg(6, erffactor);

        // coulomb kernel 2
        m_coulomb2K.setArg(0, m_oDevice);
        m_coulomb2K.setArg(1, m_sDevice);
        m_coulomb2K.setArg(2, m_qDevice);
        m_coulomb2K.setArg(4, cutoff2);
        m_coulomb2K.setArg(5, m_sDevice);
        m_coulomb2K.setArg(6, m_world.parameters().gridX);
        m_coulomb2K.setArg(7, m_world.parameters().gridY);
        m_coulomb2K.setArg(8, m_world.parameters().electrostaticPrefactor);

        // gauss kernel 2
        m_guass2K.setArg(0, m_oDevice);
        m_guass2K.setArg(1, m_sDevice);
        m_guass2K.setArg(2, m_qDevice);
        m_guass2K.setArg(4, cutoff2);
        m_guass2K.setArg(5, m_sDevice);
        m_guass2K.setArg(6, m_world.parameters().gridX);
        m_guass2K.setArg(7, m_world.parameters().gridY);
        m_guass2K.setArg(8, m_world.parameters().electrostaticPrefactor);
        m_guass2K.setArg(9, erffactor);

        //force queues to finish
        m_queue.finish();

        //should be ok to use OpenCL
        m_world.parameters().okCL = true;
    }
    catch(cl::Error& error)
    {
        qDebug("message: %s (%d)", error.what(), error.err());
        m_world.parameters().okCL = false;
        if (m_world.parameters().useOpenCL)
        {
            qFatal("message: OpenCL errors, yet use.opencl=True");
        }
        return;
    }
    m_world.parameters().okCL = true;
#endif //LANGMUIR_OPEN_CL
}

bool OpenClHelper::toggleOpenCL(bool on)
{
    if(on)
    {
        if(!(m_world.parameters().okCL))
        {
            m_world.parameters().useOpenCL = false;
            qDebug("message: can not use openCL - openCL has been turned off");
            return false;
        }
        if(!(m_world.parameters().coulombCarriers))
        {
            m_world.parameters().useOpenCL = false;
            qDebug("message: coulomb interactions are off - disabling openCL");
            return false;
        }
        m_world.parameters().useOpenCL = true;
        return true;
    }
    else
    {
        m_world.parameters().useOpenCL = false;
        return false;
    }
}

void OpenClHelper::launchCoulombKernel1()
{
#ifdef LANGMUIR_OPEN_CL
    try
    {
        int totalCharges = 0;

        //copy electrons
        for(int i = 0; i < m_world.electrons().size(); i++)
        {
            m_sHost[i+totalCharges] = m_world.electrons()[i]->getCurrentSite();
            m_qHost[i+totalCharges] = m_world.electrons()[i]->charge();
            m_world.electrons()[i]->setOpenCLID(i);
        }
        totalCharges += m_world.electrons().size();

        //copy holes
        for(int i = 0; i < m_world.holes().size(); i++)
        {
            m_sHost[i+totalCharges] = m_world.holes()[i]->getCurrentSite();
            m_qHost[i+totalCharges] = m_world.holes()[i]->charge();
            m_world.holes()[i]->setOpenCLID(i + totalCharges);
        }
        totalCharges += m_world.holes().size();

        //copy defects
        if(m_world.parameters().defectsCharge != 0)
        {
            for(int i = 0; i < m_world.defectSiteIDs().size(); i++)
            {
                m_sHost[i+totalCharges] = m_world.defectSiteIDs()[i];
                m_qHost[i+totalCharges] = m_world.parameters().defectsCharge;
            }
            totalCharges += m_world.defectSiteIDs().size();
        }
        m_offset = totalCharges;
        m_coulomb1K.setArg(3, totalCharges);

        //calculate memory sizes
        size_t sSize = totalCharges*sizeof(int);
        size_t qSize = totalCharges*sizeof(int);
        size_t oSize = totalCharges*sizeof(double);

        //calculate ranges
        cl::NDRange zSize = cl::NDRange(0, 0, 0);

        cl::NDRange gSize = cl::NDRange(
            m_world.parameters().gridX * m_world.parameters().workX,
            m_world.parameters().gridY * m_world.parameters().workY,
            m_world.parameters().gridZ * m_world.parameters().workZ);

        cl::NDRange wSize = cl::NDRange(
            m_world.parameters().workX,
            m_world.parameters().workY,
            m_world.parameters().workZ);

        //write to GPU
        m_queue.enqueueWriteBuffer(m_sDevice, CL_TRUE, 0, sSize, &m_sHost[0]);
        m_queue.enqueueWriteBuffer(m_qDevice, CL_TRUE, 0, qSize, &m_qHost[0]);

        //call kernel
        m_queue.enqueueNDRangeKernel(m_coulomb1K, zSize, gSize, wSize);

        //read from GPU
        m_queue.enqueueReadBuffer(m_oDevice, CL_TRUE, 0, oSize, &m_oHost[0]);
        m_queue.finish();
    }
    catch(cl::Error& error)
    {
        qDebug("message: %s(%d)", error.what(), error.err());
        qFatal("message: Fatal OpenCl fatal error when calling coulomb1");
        return;
    }
#endif //LANGMUIR_OPEN_CL
}

void OpenClHelper::launchGaussKernel1()
{
#ifdef LANGMUIR_OPEN_CL
    try
    {
        int totalCharges = 0;

        //copy electrons
        for(int i = 0; i < m_world.electrons().size(); i++)
        {
            m_sHost[i+totalCharges] = m_world.electrons()[i]->getCurrentSite();
            m_qHost[i+totalCharges] = m_world.electrons()[i]->charge();
            m_world.electrons()[i]->setOpenCLID(i);
        }
        totalCharges += m_world.electrons().size();

        //copy holes
        for(int i = 0; i < m_world.holes().size(); i++)
        {
            m_sHost[i+totalCharges] = m_world.holes()[i]->getCurrentSite();
            m_qHost[i+totalCharges] = m_world.holes()[i]->charge();
            m_world.holes()[i]->setOpenCLID(i + totalCharges);
        }
        totalCharges += m_world.holes().size();

        //copy defects
        if(m_world.parameters().defectsCharge != 0)
        {
            for(int i = 0; i < m_world.defectSiteIDs().size(); i++)
            {
                m_sHost[i+totalCharges] = m_world.defectSiteIDs()[i];
                m_qHost[i+totalCharges] = m_world.parameters().defectsCharge;
            }
            totalCharges += m_world.defectSiteIDs().size();
        }
        m_offset = totalCharges;
        m_guass1K.setArg(3, totalCharges);

        //calculate memory sizes
        size_t sSize = totalCharges*sizeof(int);
        size_t qSize = totalCharges*sizeof(int);
        size_t oSize = totalCharges*sizeof(double);

        //calculate ranges
        cl::NDRange zSize = cl::NDRange(0, 0, 0);

        cl::NDRange gSize = cl::NDRange(
            m_world.parameters().gridX * m_world.parameters().workX,
            m_world.parameters().gridY * m_world.parameters().workY,
            m_world.parameters().gridZ * m_world.parameters().workZ);

        cl::NDRange wSize = cl::NDRange(
            m_world.parameters().workX,
            m_world.parameters().workY,
            m_world.parameters().workZ);

        //write to GPU
        m_queue.enqueueWriteBuffer(m_sDevice, CL_TRUE, 0, sSize, &m_sHost[0]);
        m_queue.enqueueWriteBuffer(m_qDevice, CL_TRUE, 0, qSize, &m_qHost[0]);

        //call kernel
        m_queue.enqueueNDRangeKernel(m_guass1K, zSize, gSize, wSize);

        //read from GPU
        m_queue.enqueueReadBuffer(m_oDevice, CL_TRUE, 0, oSize, &m_oHost[0]);
        m_queue.finish();
    }
    catch(cl::Error& error)
    {
        qDebug("message: %s(%d)", error.what(), error.err());
        qFatal("message: Fatal OpenCl fatal error when calling gauss1");
        return;
    }
#endif //LANGMUIR_OPEN_CL
}

void OpenClHelper::launchCoulombKernel2()
{
#ifdef LANGMUIR_OPEN_CL
    try
    {
        int totalCharges = 0;

        //copy electrons
        for(int i = 0; i < m_world.electrons().size(); i++)
        {
            m_sHost[i+totalCharges] = m_world.electrons()[i]->getCurrentSite();
            m_qHost[i+totalCharges] = m_world.electrons()[i]->charge();
            m_world.electrons()[i]->setOpenCLID(i);
        }
        totalCharges += m_world.electrons().size();

        //copy holes
        for(int i = 0; i < m_world.holes().size(); i++)
        {
            m_sHost[i+totalCharges] = m_world.holes()[i]->getCurrentSite();
            m_qHost[i+totalCharges] = m_world.holes()[i]->charge();
            m_world.holes()[i]->setOpenCLID(i + totalCharges);
        }
        totalCharges += m_world.holes().size();

        //copy defects
        if(m_world.parameters().defectsCharge != 0)
        {
            for(int i = 0; i < m_world.defectSiteIDs().size(); i++)
            {
                m_sHost[i+totalCharges] = m_world.defectSiteIDs()[i];
                m_qHost[i+totalCharges] = m_world.parameters().defectsCharge;
            }
            totalCharges += m_world.defectSiteIDs().size();
        }
        m_offset = totalCharges;
        m_coulomb2K.setArg(3, totalCharges);

        //copy electrons (future)
        for(int i = 0; i < m_world.electrons().size(); i++)
        {
            m_sHost[i + totalCharges] = m_world.electrons()[i]->getFutureSite();
            m_qHost[i + totalCharges] = m_world.electrons()[i]->charge();
        }
        totalCharges += m_world.electrons().size();

        //copy holes (future)
        for(int i = 0; i < m_world.holes().size(); i++)
        {
            m_sHost[i+totalCharges] = m_world.holes()[i]->getFutureSite();
            m_qHost[i+totalCharges] = m_world.holes()[i]->charge();
        }
        totalCharges += m_world.holes().size();

        //calculate memory sizes
        size_t sSize = totalCharges*sizeof(int);
        size_t qSize = totalCharges*sizeof(int);
        size_t oSize = totalCharges*sizeof(double);

        //calculate ranges
        cl::NDRange zSize = cl::NDRange(0);

        cl::NDRange gSize = cl::NDRange(
            totalCharges * m_world.parameters().workSize);

        cl::NDRange wSize = cl::NDRange(m_world.parameters().workSize);

        //write to GPU
        m_queue.enqueueWriteBuffer(m_sDevice, CL_TRUE, 0, sSize, &m_sHost[0]);
        m_queue.enqueueWriteBuffer(m_qDevice, CL_TRUE, 0, qSize, &m_qHost[0]);

        //call kernel
        m_queue.enqueueNDRangeKernel(m_coulomb2K, zSize, gSize, wSize);

        //read from GPU
        m_queue.enqueueReadBuffer(m_oDevice, CL_TRUE, 0, oSize, &m_oHost[0]);
        m_queue.finish();
    }
    catch(cl::Error& error)
    {
        qDebug("message: %s(%d)", error.what(), error.err());
        qFatal("message: Fatal OpenCl fatal error when calling coulomb2");
        return;
    }
#endif //LANGMUIR_OPEN_CL
}

void OpenClHelper::launchGaussKernel2()
{
#ifdef LANGMUIR_OPEN_CL
    try
    {
        int totalCharges = 0;

        //copy electrons
        for(int i = 0; i < m_world.electrons().size(); i++)
        {
            m_sHost[i+totalCharges] = m_world.electrons()[i]->getCurrentSite();
            m_qHost[i+totalCharges] = m_world.electrons()[i]->charge();
            m_world.electrons()[i]->setOpenCLID(i);
        }
        totalCharges += m_world.electrons().size();

        //copy holes
        for(int i = 0; i < m_world.holes().size(); i++)
        {
            m_sHost[i+totalCharges] = m_world.holes()[i]->getCurrentSite();
            m_qHost[i+totalCharges] = m_world.holes()[i]->charge();
            m_world.holes()[i]->setOpenCLID(i + totalCharges);
        }
        totalCharges += m_world.holes().size();

        //copy defects
        if(m_world.parameters().defectsCharge != 0)
        {
            for(int i = 0; i < m_world.defectSiteIDs().size(); i++)
            {
                m_sHost[i+totalCharges] = m_world.defectSiteIDs()[i];
                m_qHost[i+totalCharges] = m_world.parameters().defectsCharge;
            }
            totalCharges += m_world.defectSiteIDs().size();
        }
        m_offset = totalCharges;
        m_guass2K.setArg(3, totalCharges);

        //copy electrons (future)
        for(int i = 0; i < m_world.electrons().size(); i++)
        {
            m_sHost[i + totalCharges] = m_world.electrons()[i]->getFutureSite();
            m_qHost[i + totalCharges] = m_world.electrons()[i]->charge();
        }
        totalCharges += m_world.electrons().size();

        //copy holes (future)
        for(int i = 0; i < m_world.holes().size(); i++)
        {
            m_sHost[i+totalCharges] = m_world.holes()[i]->getFutureSite();
            m_qHost[i+totalCharges] = m_world.holes()[i]->charge();
        }
        totalCharges += m_world.holes().size();

        //calculate memory sizes
        size_t sSize = totalCharges*sizeof(int);
        size_t qSize = totalCharges*sizeof(int);
        size_t oSize = totalCharges*sizeof(double);

        //calculate ranges
        cl::NDRange zSize = cl::NDRange(0);

        cl::NDRange gSize = cl::NDRange(
            totalCharges * m_world.parameters().workSize);

        cl::NDRange wSize = cl::NDRange(m_world.parameters().workSize);

        //write to GPU
        m_queue.enqueueWriteBuffer(m_sDevice, CL_TRUE, 0, sSize, &m_sHost[0]);
        m_queue.enqueueWriteBuffer(m_qDevice, CL_TRUE, 0, qSize, &m_qHost[0]);

        //call kernel
        m_queue.enqueueNDRangeKernel(m_guass2K, zSize, gSize, wSize);

        //read from GPU
        m_queue.enqueueReadBuffer(m_oDevice, CL_TRUE, 0, oSize, &m_oHost[0]);
        m_queue.finish();
    }
    catch(cl::Error& error)
    {
        qDebug("message: %s(%d)", error.what(), error.err());
        qFatal("message: Fatal OpenCl fatal error when calling gauss2");
        return;
    }
#endif //LANGMUIR_OPEN_CL
}

void OpenClHelper::compareHostAndDeviceForAllCarriers()
{
#ifdef LANGMUIR_OPEN_CL
    foreach(ChargeAgent *charge, m_world.electrons())
    {
        charge->compareCoulomb();
    }
    foreach(ChargeAgent *charge, m_world.holes())
    {
        charge->compareCoulomb();
    }
#endif //LANGMUIR_OPEN_CL
}

void OpenClHelper::copySiteAndChargeToHostVector(int index, int site, int charge)
{
#ifdef LANGMUIR_OPEN_CL
    m_sHost[index] = site; m_qHost[index] = charge;
#endif //LANGMUIR_OPEN_CL
}

double OpenClHelper::getOutputHost(int index) const
{
#ifdef LANGMUIR_OPEN_CL
    return m_oHost[index];
#else
    return 0;
#endif //LANGMUIR_OPEN_CL
}

double OpenClHelper::getOutputHostFuture(int index) const
{
#ifdef LANGMUIR_OPEN_CL
    return m_oHost[index+m_offset];
#else
    return 0;
#endif //LANGMUIR_OPEN_CL
}

}
