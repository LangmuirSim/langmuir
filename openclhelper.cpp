#include <QTextStream>

#include "openclhelper.h"
#include "chargeagent.h"
#include "parameters.h"
#include "cubicgrid.h"
#include "potential.h"
#include "world.h"

namespace Langmuir
{

OpenClHelper::OpenClHelper(World &world, QObject *parent): QObject(parent), m_world(world)
{
}

void OpenClHelper::initializeOpenCL()
{
    //can't use openCL yet
    m_world.parameters().okCL = false;

#ifdef LANGMUIR_OPEN_CL
    try
    {
        //error code
        cl_int err;

        //obtain platforms
        cl::vector<cl::Platform> platforms;
        err = cl::Platform::get(&platforms);
        if(platforms.size()<= 0){ throw cl::Error(-1, "can not find any platforms!"); }

        //obtain devices
        cl::vector<cl::Device> devices;
        err = platforms[0].getDevices(CL_DEVICE_TYPE_GPU, &devices);
        if(devices.size()<= 0){ throw cl::Error(-1, "can not find any devices!"); }

        //obtain context
        cl_context_properties contextProperties[3] =
        { CL_CONTEXT_PLATFORM,(cl_context_properties)platforms[0](), 0 };
        m_context = cl::Context(devices, contextProperties, NULL, NULL, &err);

        //obtain command queue
        m_queue = cl::CommandQueue(m_context, devices[0], 0, &err);
        err = m_queue.finish();

        //obtain kernel source
        QFile file(":/resources/kernel.cl");
        if(!(file.open(QIODevice::ReadOnly | QIODevice::Text)) ){
            qDebug() << qPrintable(QString("error opening kernel file: :/resources.kernel.cl;\n\t"
                                           "this is a qt4.qrc file, somethings seriously wrong!"));
            throw cl::Error(-1, "OpenCL Error!");
        }
        QByteArray contents = file.readAll();
        file.close();
        cl::Program::Sources source(1, std::make_pair(contents, contents.size()));
        cl::Program program(m_context, source, &err);
        err = program.build(devices, NULL, NULL, NULL);
        m_coulomb1K = cl::Kernel(program, "coulomb1", &err);
        m_guass1K   = cl::Kernel(program, "gauss1"  , &err);
        m_coulomb2K = cl::Kernel(program, "coulomb2", &err);
        m_guass2K   = cl::Kernel(program, "gauss2"  , &err);

        //initialize Host Memory
        m_sHost.clear();
        m_qHost.clear();
        m_oHost.clear();
        m_sHost.resize(m_world.electronGrid().volume());
        m_qHost.resize(m_world.electronGrid().volume());
        m_oHost.resize(m_world.electronGrid().volume());

        //initialize Device Memory
        m_sDevice = cl::Buffer(m_context, CL_MEM_READ_ONLY, m_sHost.size()* sizeof(int), NULL, &err);
        m_qDevice = cl::Buffer(m_context, CL_MEM_READ_ONLY, m_qHost.size()* sizeof(int), NULL, &err);
        m_oDevice = cl::Buffer(m_context, CL_MEM_READ_WRITE, m_oHost.size()* sizeof(double), NULL, &err);

        //upload initial data
        err = m_queue.enqueueWriteBuffer(m_sDevice, CL_TRUE, 0, m_sHost.size()* sizeof(int), &m_sHost[0], NULL, NULL);
        err = m_queue.enqueueWriteBuffer(m_sDevice, CL_TRUE, 0, m_qHost.size()* sizeof(int), &m_qHost[0], NULL, NULL);
        err = m_queue.enqueueWriteBuffer(m_oDevice, CL_TRUE, 0, m_oHost.size()* sizeof(double), &m_oHost[0], NULL, NULL);

        //preset kernel arguments that dont change

        // coulomb kernel 1
        err = m_coulomb1K.setArg(0, m_oDevice);
        err = m_coulomb1K.setArg(1, m_sDevice);
        err = m_coulomb1K.setArg(2, m_qDevice);
        err = m_coulomb1K.setArg(4, m_world.parameters().electrostaticCutoff *
                                    m_world.parameters().electrostaticCutoff);
        err = m_coulomb1K.setArg(5, m_world.parameters().electrostaticPrefactor);

        // gauss kernel 1
        double erffactor = 0.0;
        if (m_world.parameters().coulombGaussianSigma > 0)
        {
            erffactor = 1.0 / (sqrt(2.0) * m_world.parameters().coulombGaussianSigma);
        }
        err = m_guass1K.setArg(0, m_oDevice);
        err = m_guass1K.setArg(1, m_sDevice);
        err = m_guass1K.setArg(2, m_qDevice);
        err = m_guass1K.setArg(4, m_world.parameters().electrostaticCutoff *
                                  m_world.parameters().electrostaticCutoff);
        err = m_guass1K.setArg(5, m_world.parameters().electrostaticPrefactor);
        err = m_guass1K.setArg(6, erffactor);

        // coulomb kernel 2
        err = m_coulomb2K.setArg(0, m_oDevice);
        err = m_coulomb2K.setArg(1, m_sDevice);
        err = m_coulomb2K.setArg(2, m_qDevice);
        err = m_coulomb2K.setArg(4, m_world.parameters().electrostaticCutoff *
                                    m_world.parameters().electrostaticCutoff);
        err = m_coulomb2K.setArg(5, m_sDevice);
        err = m_coulomb2K.setArg(6, m_world.parameters().gridX);
        err = m_coulomb2K.setArg(7, m_world.parameters().gridY);
        err = m_coulomb2K.setArg(8, m_world.parameters().electrostaticPrefactor);

        // gauss kernel 2
        err = m_guass2K.setArg(0, m_oDevice);
        err = m_guass2K.setArg(1, m_sDevice);
        err = m_guass2K.setArg(2, m_qDevice);
        err = m_guass2K.setArg(4, m_world.parameters().electrostaticCutoff *
                                    m_world.parameters().electrostaticCutoff);
        err = m_guass2K.setArg(5, m_sDevice);
        err = m_guass2K.setArg(6, m_world.parameters().gridX);
        err = m_guass2K.setArg(7, m_world.parameters().gridY);
        err = m_guass2K.setArg(8, m_world.parameters().electrostaticPrefactor);
        err = m_guass2K.setArg(9, erffactor);

        //check for errors
        {
            int maxCharges = m_world.maxChargeAgentsAndChargedDefects();

            //kernel1
            int wx1 = m_world.parameters().workX;
            int wy1 = m_world.parameters().workY;
            int wz1 = m_world.parameters().workZ;
            int wv1 = wx1 * wy1 * wz1;
            int gx1 = wx1 * m_world.parameters().gridX;
            int gy1 = wy1 * m_world.parameters().gridY;
            int gz1 = wz1 * m_world.parameters().gridZ;
            int gv1 = gx1 * gy1 * gz1;

            //kernel2
            int wx2 = m_world.parameters().workSize;
            int wv2 = wx2;
            int gx2 = wx2 * maxCharges;
            int gv2 = gx2;

            //device
            cl::vector<size_t> gxyz_d = devices[0].getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>(&err);
            int gv_d = gxyz_d[0] * gxyz_d[1] * gxyz_d[2];
            int wv1_d = m_coulomb1K.getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(devices[0], &err);
            int wv2_d = m_coulomb2K.getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(devices[0], &err);

            QString deviceQuery = "";
            double ilog2 = 1.0 / log(2);
            int space = 12;
            deviceQuery += QString("       %1 %2 %3 : %4 %5 %6 : %7\n")
                    .arg("X", space)
                    .arg("Y", space)
                    .arg("Z", space)
                    .arg("Log2X", space)
                    .arg("Log2Y", space)
                    .arg("Log2Z", space)
                    .arg("V", space);
            deviceQuery += QString("\nKERNEL1\n");
            deviceQuery += QString("W    : %1 %2 %3 : %4 %5 %6 : %7\n")
                    .arg(wx1, space)
                    .arg(wy1, space)
                    .arg(wz1, space)
                    .arg(log(wx1)*ilog2, space)
                    .arg(log(wy1)*ilog2, space)
                    .arg(log(wz1)*ilog2, space)
                    .arg(wv1, space);
            deviceQuery += QString("G    : %1 %2 %3 : %4 %5 %6 : %7\n")
                    .arg(gx1, space)
                    .arg(gy1, space)
                    .arg(gz1, space)
                    .arg(log(gx1)*ilog2, space)
                    .arg(log(gy1)*ilog2, space)
                    .arg(log(gz1)*ilog2, space)
                    .arg(gv1, space);
            deviceQuery += QString("\nKERNEL2\n");
            deviceQuery += QString("W    : %1 %2 %3 : %4 %5 %6 : %7\n")
                    .arg(wx2, space)
                    .arg(1, space)
                    .arg(1, space)
                    .arg(log(wx2)*ilog2, space)
                    .arg(0, space)
                    .arg(0, space)
                    .arg(wv2, space);
            deviceQuery += QString("G    : %1 %2 %3 : %4 %5 %6 : %7\n")
                    .arg(gx2, space)
                    .arg(1, space)
                    .arg(1, space)
                    .arg(log(gx2)*ilog2, space)
                    .arg(0, space)
                    .arg(0, space)
                    .arg(gv2, space);
            deviceQuery += QString("\nDEVICE\n");
            deviceQuery += QString("W1   : %1\n").arg(wv1_d, space);
            deviceQuery += QString("W2   : %1\n").arg(wv2_d, space);
            deviceQuery += QString("G    : %1 %2 %3 : %4 %5 %6 : %7\n")
                    .arg(gxyz_d[0], space)
                    .arg(gxyz_d[1], space)
                    .arg(gxyz_d[2], space)
                    .arg(log(gxyz_d[0])*ilog2, space)
                    .arg(log(gxyz_d[1])*ilog2, space)
                    .arg(log(gxyz_d[2])*ilog2, space)
                    .arg(gv_d, space);
            deviceQuery += QString("\nOTHER\n");
            deviceQuery += QString("MAX  : %1\n").arg(maxCharges, space);
            deviceQuery += QString("GRID : %1 %2 %3 : %4 %5 %6 : %7\n")
                    .arg(m_world.parameters().gridX, space)
                    .arg(m_world.parameters().gridY, space)
                    .arg(m_world.parameters().gridZ, space)
                    .arg(log(m_world.parameters().gridX)*ilog2, space)
                    .arg(log(m_world.parameters().gridY)*ilog2, space)
                    .arg(log(m_world.parameters().gridZ)*ilog2, space)
                    .arg(m_world.electronGrid().volume(), space);

            //checks
            if(wv2 > wv2_d)
            {
                qDebug()<< qPrintable(deviceQuery);
                qFatal("work group volume of coulomb kernel 2 exceeds maxiumum for device");
            }

            if(wv1 > wv1_d && m_world.parameters().outputCoulomb)
            {
                qDebug()<< qPrintable(deviceQuery);
                qFatal("work group volume of coulomb kernel 1 exceeds maxiumum for device");
            }

            if(gv1 > gv_d && m_world.parameters().outputCoulomb)
            {
                qDebug()<< qPrintable(deviceQuery);
                qFatal("global work item count of coulomb kernel 1 exceeds maximum for device");
            }

            if(gv2 > gv_d)
            {
                qDebug()<< qPrintable(deviceQuery);
                qFatal("global work item count of coulomb kernel 1 exceeds maximum for device");
            }

            int bytes =(sizeof(int)+ sizeof(int)+ sizeof(double)) * maxCharges;
            int maxMemory = devices[0].getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>(&err);
            if(bytes > maxMemory)
            {
                qWarning(" may exceed device memory - expecting crash");
            }
        }
        //force queues to finish
        err = m_queue.finish();

        //should be ok to use OpenCL
        m_world.parameters().okCL = true;
    }
    catch(cl::Error& error)
    {
        qDebug("%s (%d)", error.what(), error.err());
        m_world.parameters().okCL = false;
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
            qDebug()<< "can not use openCL on this platform - openCL has been turned off";
            return false;
        }
        if(!(m_world.parameters().coulombCarriers))
        {
            m_world.parameters().useOpenCL = false;
            qDebug()<< "OpenCL is on but coulomb interactions are off - disabling openCL";
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
        //copy charges
        for(int i = 0; i < m_world.electrons().size(); i++)
        {
            m_sHost[i+totalCharges] = m_world.electrons().at(i)->getCurrentSite();
            m_qHost[i+totalCharges] = m_world.electrons().at(i)->charge();
            m_world.electrons().at(i)->setOpenCLID(i);
        }
        totalCharges += m_world.electrons().size();

        //copy holes
        for(int i = 0; i < m_world.holes().size(); i++)
        {
            m_sHost[i+totalCharges] = m_world.holes().at(i)->getCurrentSite();
            m_qHost[i+totalCharges] = m_world.holes().at(i)->charge();
            m_world.holes().at(i)->setOpenCLID(i+totalCharges);
        }
        totalCharges += m_world.holes().size();

        //copy defects
        if(m_world.parameters().defectsCharge != 0)
        {
            for(int i = 0; i < m_world.defectSiteIDs().size(); i++)
            {
                m_sHost[i+totalCharges] = m_world.defectSiteIDs().at(i);
                m_qHost[i+totalCharges] = m_world.parameters().defectsCharge;
            }
            totalCharges += m_world.defectSiteIDs().size();
        }
        m_offset = totalCharges;
        m_coulomb1K.setArg(3, totalCharges);
        m_queue.enqueueWriteBuffer(m_sDevice, CL_TRUE, 0, totalCharges*sizeof(int), &m_sHost[0]);
        m_queue.enqueueWriteBuffer(m_qDevice, CL_TRUE, 0, totalCharges*sizeof(int), &m_qHost[0]);
        m_queue.enqueueNDRangeKernel(m_coulomb1K, cl::NDRange(0, 0, 0),
                                     cl::NDRange(m_world.parameters().gridX * m_world.parameters().workX,
                                                 m_world.parameters().gridY * m_world.parameters().workY,
                                                 m_world.parameters().gridZ * m_world.parameters().workZ),
                                     cl::NDRange(m_world.parameters().workX,
                                                 m_world.parameters().workY,
                                                 m_world.parameters().workZ));
        m_queue.enqueueReadBuffer(m_oDevice, CL_TRUE, 0, m_oHost.size()*sizeof(double), &m_oHost[0]);
        m_queue.finish();
    }
    catch(cl::Error& error)
    {
        qWarning("%s(%d)", error.what(), error.err());
        qFatal("Fatal OpenCl fatal error when calling coulomb1");
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
        //copy charges
        for(int i = 0; i < m_world.electrons().size(); i++)
        {
            m_sHost[i+totalCharges] = m_world.electrons().at(i)->getCurrentSite();
            m_qHost[i+totalCharges] = m_world.electrons().at(i)->charge();
            m_world.electrons().at(i)->setOpenCLID(i);
        }
        totalCharges += m_world.electrons().size();

        //copy holes
        for(int i = 0; i < m_world.holes().size(); i++)
        {
            m_sHost[i+totalCharges] = m_world.holes().at(i)->getCurrentSite();
            m_qHost[i+totalCharges] = m_world.holes().at(i)->charge();
            m_world.holes().at(i)->setOpenCLID(i+totalCharges);
        }
        totalCharges += m_world.holes().size();

        //copy defects
        if(m_world.parameters().defectsCharge != 0)
        {
            for(int i = 0; i < m_world.defectSiteIDs().size(); i++)
            {
                m_sHost[i+totalCharges] = m_world.defectSiteIDs().at(i);
                m_qHost[i+totalCharges] = m_world.parameters().defectsCharge;
            }
            totalCharges += m_world.defectSiteIDs().size();
        }
        m_offset = totalCharges;
        m_guass1K.setArg(3, totalCharges);
        m_queue.enqueueWriteBuffer(m_sDevice, CL_TRUE, 0, totalCharges*sizeof(int), &m_sHost[0]);
        m_queue.enqueueWriteBuffer(m_qDevice, CL_TRUE, 0, totalCharges*sizeof(int), &m_qHost[0]);
        m_queue.enqueueNDRangeKernel(m_guass1K, cl::NDRange(0, 0, 0),
                                     cl::NDRange(m_world.parameters().gridX * m_world.parameters().workX,
                                                 m_world.parameters().gridY * m_world.parameters().workY,
                                                 m_world.parameters().gridZ * m_world.parameters().workZ),
                                     cl::NDRange(m_world.parameters().workX,
                                                 m_world.parameters().workY,
                                                 m_world.parameters().workZ));
        m_queue.enqueueReadBuffer(m_oDevice, CL_TRUE, 0, m_oHost.size()*sizeof(double), &m_oHost[0]);
        m_queue.finish();
    }
    catch(cl::Error& error)
    {
        qWarning("%s(%d)", error.what(), error.err());
        qFatal("Fatal OpenCl fatal error when calling gauss1");
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
            m_sHost[i + totalCharges] = m_world.electrons().at(i)->getCurrentSite();
            m_qHost[i + totalCharges] = m_world.electrons().at(i)->charge();
            m_world.electrons().at(i)->setOpenCLID(i);
        }
        totalCharges += m_world.electrons().size();

        //copy holes
        for(int i = 0; i < m_world.holes().size(); i++)
        {
            m_sHost[i + totalCharges] = m_world.holes().at(i)->getCurrentSite();
            m_qHost[i + totalCharges] = m_world.holes().at(i)->charge();
            m_world.holes().at(i)->setOpenCLID(i+totalCharges);
        }
        totalCharges += m_world.holes().size();

        //copy defects
        if(m_world.parameters().defectsCharge != 0)
        {
            for(int i = 0; i < m_world.defectSiteIDs().size(); i++)
            {
                m_sHost[i + totalCharges] = m_world.defectSiteIDs().at(i);
                m_qHost[i + totalCharges] = m_world.parameters().defectsCharge;
            }
            totalCharges += m_world.defectSiteIDs().size();
        }

        //set this argument here for future/past reasons
        m_offset = totalCharges;
        m_coulomb2K.setArg(3, totalCharges);

        //copy charges(future)
        for(int i = 0; i < m_world.electrons().size(); i++)
        {
            m_sHost[i + totalCharges] = m_world.electrons().at(i)->getFutureSite();
            m_qHost[i + totalCharges] = m_world.electrons().at(i)->charge();
        }
        totalCharges += m_world.electrons().size();

        //You added this recently
        for(int i = 0; i < m_world.holes().size(); i++)
        {
            m_sHost[i+totalCharges] = m_world.holes().at(i)->getFutureSite();
            m_qHost[i+totalCharges] = m_world.holes().at(i)->charge();
        }
        totalCharges += m_world.holes().size();

        m_queue.enqueueWriteBuffer(m_sDevice, CL_TRUE, 0,(totalCharges)*sizeof(int), &m_sHost[0]);
        m_queue.enqueueWriteBuffer(m_qDevice, CL_TRUE, 0,(totalCharges)*sizeof(int), &m_qHost[0]);
        m_queue.enqueueNDRangeKernel(m_coulomb2K,
                                     cl::NDRange(0),
                                     cl::NDRange(totalCharges*m_world.parameters().workSize),
                                     cl::NDRange(m_world.parameters().workSize));
        m_queue.enqueueReadBuffer(m_oDevice, CL_TRUE, 0, (totalCharges)*sizeof(double), &m_oHost[0]);
        m_queue.finish();
    }
    catch(cl::Error& error)
    {
        qWarning("%s(%d)", error.what(), error.err());
        qFatal("Fatal OpenCl fatal error when calling coulomb2");
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
            m_sHost[i + totalCharges] = m_world.electrons().at(i)->getCurrentSite();
            m_qHost[i + totalCharges] = m_world.electrons().at(i)->charge();
            m_world.electrons().at(i)->setOpenCLID(i);
        }
        totalCharges += m_world.electrons().size();

        //copy holes
        for(int i = 0; i < m_world.holes().size(); i++)
        {
            m_sHost[i + totalCharges] = m_world.holes().at(i)->getCurrentSite();
            m_qHost[i + totalCharges] = m_world.holes().at(i)->charge();
            m_world.holes().at(i)->setOpenCLID(i+totalCharges);
        }
        totalCharges += m_world.holes().size();

        //copy defects
        if(m_world.parameters().defectsCharge != 0)
        {
            for(int i = 0; i < m_world.defectSiteIDs().size(); i++)
            {
                m_sHost[i + totalCharges] = m_world.defectSiteIDs().at(i);
                m_qHost[i + totalCharges] = m_world.parameters().defectsCharge;
            }
            totalCharges += m_world.defectSiteIDs().size();
        }

        //set this argument here for future/past reasons
        m_offset = totalCharges;
        m_guass2K.setArg(3, totalCharges);

        //copy charges(future)
        for(int i = 0; i < m_world.electrons().size(); i++)
        {
            m_sHost[i + totalCharges] = m_world.electrons().at(i)->getFutureSite();
            m_qHost[i + totalCharges] = m_world.electrons().at(i)->charge();
        }
        totalCharges += m_world.electrons().size();

        //You added this recently
        for(int i = 0; i < m_world.holes().size(); i++)
        {
            m_sHost[i+totalCharges] = m_world.holes().at(i)->getFutureSite();
            m_qHost[i+totalCharges] = m_world.holes().at(i)->charge();
        }
        totalCharges += m_world.holes().size();

        m_queue.enqueueWriteBuffer(m_sDevice, CL_TRUE, 0,(totalCharges)*sizeof(int), &m_sHost[0]);
        m_queue.enqueueWriteBuffer(m_qDevice, CL_TRUE, 0,(totalCharges)*sizeof(int), &m_qHost[0]);
        m_queue.enqueueNDRangeKernel(m_guass2K,
                                     cl::NDRange(0),
                                     cl::NDRange(totalCharges*m_world.parameters().workSize),
                                     cl::NDRange(m_world.parameters().workSize));
        m_queue.enqueueReadBuffer(m_oDevice, CL_TRUE, 0, (totalCharges)*sizeof(double), &m_oHost[0]);
        m_queue.finish();
    }
    catch(cl::Error& error)
    {
        qWarning("%s(%d)", error.what(), error.err());
        qFatal("Fatal OpenCl fatal error when calling gauss2");
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
