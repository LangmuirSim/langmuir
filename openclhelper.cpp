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
        QDir dir(m_world.parameters().kernelsPath);
        if(!dir.exists()) { qFatal("%s does not exist", qPrintable(dir.path()));}
        QFile file(dir.absoluteFilePath("kernel.cl"));
        if(!(file.open(QIODevice::ReadOnly | QIODevice::Text)) ){
            throw cl::Error(-1, QString("error opening kernel file: %1").arg(dir.absoluteFilePath("kernel.cl")).toLatin1());
        }
        QByteArray contents = file.readAll();
        file.close();
        cl::Program::Sources source(1, std::make_pair(contents, contents.size()));
        cl::Program program(m_context, source, &err);
        err = program.build(devices, NULL, NULL, NULL);
        m_coulombK1 = cl::Kernel(program, "coulomb1", &err);
        m_coulombK2 = cl::Kernel(program, "coulomb2", &err);

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
        err = m_coulombK1.setArg(0, m_oDevice);
        err = m_coulombK1.setArg(1, m_sDevice);
        err = m_coulombK1.setArg(2, m_qDevice);
        err = m_coulombK1.setArg(4, m_world.parameters().electrostaticCutoff * m_world.parameters().electrostaticCutoff);
        err = m_coulombK1.setArg(5, m_world.parameters().electrostaticPrefactor);

        err = m_coulombK2.setArg(0, m_oDevice);
        err = m_coulombK2.setArg(1, m_sDevice);
        err = m_coulombK2.setArg(2, m_qDevice);
        err = m_coulombK2.setArg(4, m_world.parameters().electrostaticCutoff * m_world.parameters().electrostaticCutoff);
        err = m_coulombK2.setArg(5, m_sDevice);
        err = m_coulombK2.setArg(6, m_world.parameters().gridX);
        err = m_coulombK2.setArg(7, m_world.parameters().gridY);
        err = m_coulombK2.setArg(8, m_world.parameters().electrostaticPrefactor);

        //check for errors
        {
            int maxCharges = m_world.electronGrid().volume()*(m_world.parameters().electronPercentage + m_world.parameters().defectPercentage);

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
            int wv1_d = m_coulombK1.getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(devices[0], &err);
            int wv2_d = m_coulombK2.getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(devices[0], &err);

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
        qWarning("%s(%d)", error.what(), error.err());
        m_world.parameters().okCL = false;
        return;
    }
    m_world.parameters().okCL = true;
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
        if(m_world.parameters().coulombDefects)
        {
            for(int i = 0; i < m_world.defectSiteIDs().size(); i++)
            {
                m_sHost[i+totalCharges] = m_world.defectSiteIDs().at(i);
                m_qHost[i+totalCharges] = m_world.parameters().defectsCharge;
            }
            totalCharges += m_world.defectSiteIDs().size();
        }
        m_offset = totalCharges;
        m_coulombK1.setArg(3, totalCharges);
        m_queue.enqueueWriteBuffer(m_sDevice, CL_TRUE, 0, totalCharges*sizeof(int), &m_sHost[0]);
        m_queue.enqueueWriteBuffer(m_qDevice, CL_TRUE, 0, totalCharges*sizeof(int), &m_qHost[0]);
        m_queue.enqueueNDRangeKernel(m_coulombK1, cl::NDRange(0, 0, 0), 
                                     cl::NDRange(m_world.parameters().gridX * m_world.parameters().workX,
                                                  m_world.parameters().gridY* m_world.parameters().workY,
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
        qFatal("Fatal OpenCl fatal error when calling coulomb kernel 1");
        return;
    }
}

void OpenClHelper::launchCoulombKernel2()
{
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
        if(m_world.parameters().coulombDefects)
        {
            for(int i = 0; i < m_world.defectSiteIDs().size(); i++)
            {
                m_sHost[i+totalCharges] = m_world.defectSiteIDs().at(i);
                m_qHost[i+totalCharges] = m_world.parameters().defectsCharge;
            }
            totalCharges += m_world.defectSiteIDs().size();
        }

        //set this argument here for future/past reasons
        m_offset = totalCharges;
        m_coulombK2.setArg(3, totalCharges);

        //copy charges(future)
        for(int i = 0; i < m_world.electrons().size(); i++)
        {
            m_sHost[i+totalCharges] = m_world.electrons().at(i)->getFutureSite();
            m_qHost[i+totalCharges] = m_world.electrons().at(i)->charge();
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
        m_queue.enqueueNDRangeKernel(m_coulombK2, cl::NDRange(0), cl::NDRange(totalCharges*m_world.parameters().workSize), cl::NDRange(m_world.parameters().workSize));
        m_queue.enqueueReadBuffer(m_oDevice, CL_TRUE, 0, m_oHost.size()*sizeof(double), &m_oHost[0]);
        m_queue.finish();
    }
    catch(cl::Error& error)
    {
        qWarning("%s(%d)", error.what(), error.err());
        qFatal("Fatal OpenCl fatal error when calling coulomb kernel 2");
        return;
    }
}

void OpenClHelper::compareHostAndDeviceForCarrier(int i, QList< ChargeAgent * > &charges)
{
    Grid &grid = m_world.electronGrid();
    Potential &potential = m_world.potential();
    ChargeAgent& charge = *charges.at(i);

    int F  = charge.getFutureSite();
    int S  = charge.getCurrentSite();
    int XI = grid.getIndexX(S);
    int YI = grid.getIndexY(S);
    int ZI = grid.getIndexZ(S);
    int XF = grid.getIndexX(F);
    int YF = grid.getIndexY(F);
    int ZF = grid.getIndexZ(F);
    int Q  = charge.charge();
    int CL = charge.getOpenCLID();
    int OF = m_offset;

    double PE1CPU = potential.coulombPotentialElectrons(S);
    double PH1CPU = potential.coulombPotentialHoles(S);
    double PD1CPU = potential.coulombPotentialDefects(S);

    double PE2CPU = potential.coulombPotentialElectrons(F);
    double PH2CPU = potential.coulombPotentialHoles(F);
    double PD2CPU = potential.coulombPotentialDefects(F);

    double PSI    = Q * m_world.interactionEnergies()[1][0][0];

    double P1CPU  = PE1CPU + PH1CPU + PD1CPU;
    double P2CPU  = PE2CPU + PH2CPU + PD2CPU - PSI;

    double P1GPU  = getOutputHost(CL);
    double P2GPU  = getOutputHostFuture(CL)- PSI;

    double PDIFF1 = fabs(P1CPU - P1GPU)/(0.5 *(fabs(P1CPU)+ fabs(P1GPU)) )* 100.0;
    double PDIFF2 = fabs(P2CPU - P2GPU)/(0.5 *(fabs(P2CPU)+ fabs(P2GPU)) )* 100.0;

    double DCPU   = Q *(P2CPU - P1CPU);
    double DGPU   = Q *(P2GPU - P1GPU);
    double PDIFF3 = fabs(DCPU -  DGPU)/(0.5 *(fabs(DCPU)+ fabs(DGPU)) )* 100.0;

    int DW = 13;
    int DP =  5;

    QString SS = QString("     I[%1](   (%2)  (%3)  (%4)        %5)")
            .arg(S, DW)
            .arg(XI, DW)
            .arg(YI, DW)
            .arg(ZI, DW)
            .arg("", DW);

    QString SF = QString("     F[%1](   (%2)  (%3)  (%4)        %5)")
            .arg(F, DW)
            .arg(XF, DW)
            .arg(YF, DW)
            .arg(ZF, DW)
            .arg("", DW);

    QString SP1CPU = QString("CPU P1[%1]( E:(%2)H:(%3)D:(%4)   SI:(%5))")
            .arg(P1CPU, DW, 'f', DP)
            .arg(PE1CPU, DW, 'f', DP)
            .arg(PH1CPU, DW, 'f', DP)
            .arg(PD1CPU, DW, 'f', DP)
            .arg(0.0, DW, 'f', DP);

    QString SP2CPU = QString("CPU P2[%1]( E:(%2)H:(%3)D:(%4)   SI:(%5))")
            .arg(P2CPU, DW, 'f', DP)
            .arg(PE2CPU, DW, 'f', DP)
            .arg(PH2CPU, DW, 'f', DP)
            .arg(PD2CPU, DW, 'f', DP)
            .arg(PSI, DW, 'f', DP);

    QString SP1GPU = QString("GPU P1[%1](CL:(%2)%3SI:(%4)) DIFF:(%5%)")
            .arg(P1GPU, DW, 'f', DP)
            .arg(CL, DW)
            .arg("", 2*DW + 11)
            .arg(0.0, DW, 'f', DP)
            .arg(PDIFF1, DW, 'f', DP);

    QString SP2GPU = QString("GPU P2[%1](CL:(%2)%3SI:(%4)) DIFF:(%5%)")
            .arg(P2GPU, DW, 'f', DP)
            .arg(CL+OF, DW)
            .arg("", 2*DW + 11)
            .arg(PSI, DW, 'f', DP)
            .arg(PDIFF2, DW, 'f', DP);

    QString SDGPU  = QString("DELTA [%1]( C:(%2)G:(%3)%4 ) DIFF:(%5%)")
            .arg("", DW)
            .arg(DCPU, DW)
            .arg(DGPU, DW)
            .arg("", 2*DW + 11)
            .arg(PDIFF3, DW, 'f', DP);

    if(PDIFF1 > 1e-4 || PDIFF2 > 1e-4 || PDIFF3 > 1e-4)
    {
        if(charges == m_world.electrons())
        {
            qDebug()<< "electron" << i;
        }
        else
        {
            qDebug()<< "hole" << i;
        }
        qDebug()<< qPrintable(SS);
        qDebug()<< qPrintable(SF);
        qDebug()<< qPrintable(SP1CPU);
        qDebug()<< qPrintable(SP2CPU);
        qDebug()<< qPrintable(SP1GPU);
        qDebug()<< qPrintable(SP2GPU);
        qDebug()<< qPrintable(SDGPU);
        qDebug()<< "";
    }
}

void OpenClHelper::compareHostAndDeviceAtSite(int i)
{
    Grid &grid = m_world.electronGrid();
    Potential &potential = m_world.potential();

    int XI = grid.getIndexX(i);
    int YI = grid.getIndexY(i);
    int ZI = grid.getIndexZ(i);
    int DW = 13;
    int DP =  5;

    double PE1CPU = potential.coulombPotentialElectrons(i);
    double PH1CPU = potential.coulombPotentialHoles(i);
    double PD1CPU = potential.coulombPotentialDefects(i);
    double P1CPU  = PE1CPU + PH1CPU + PD1CPU;
    double P1GPU  = getOutputHost(i);
    double PDIFF1 = fabs(P1CPU - P1GPU)/(0.5 *(fabs(P1CPU)+ fabs(P1GPU)) )* 100.0;

    QString SS = QString("     I[%1]((%2)(%3)(%4)%5)")
            .arg(i, DW)
            .arg(XI, DW)
            .arg(YI, DW)
            .arg(ZI, DW)
            .arg("", DW);

    QString SP1CPU = QString("CPU P1[%1](E:(%2)H:(%3)D:(%4)SI:(%5))")
            .arg(P1CPU, DW, 'f', DP)
            .arg(PE1CPU, DW, 'f', DP)
            .arg(PH1CPU, DW, 'f', DP)
            .arg(PD1CPU, DW, 'f', DP)
            .arg(0.0, DW, 'f', DP);

    QString SP1GPU = QString("GPU P1[%1](CL:(%2)%3SI:(%4)) DIFF:(%5%)")
            .arg(P1GPU, DW, 'f', DP)
            .arg("", DW)
            .arg("", 2*DW + 11)
            .arg(0.0, DW, 'f', DP)
            .arg(PDIFF1, DW, 'f', DP);

    qDebug()<< qPrintable(SS);
    qDebug()<< qPrintable(SP1CPU);
    qDebug()<< qPrintable(SP1GPU);
}

void OpenClHelper::compareHostAndDeviceForAllCarriers()
{
    for(int j = 0; j < m_world.electrons().size(); j++)
    {
        compareHostAndDeviceForCarrier(j,m_world.electrons());
    }
    for(int j = 0; j < m_world.holes().size(); j++)
    {
        compareHostAndDeviceForCarrier(j,m_world.holes());
    }
}

void OpenClHelper::copySiteAndChargeToHostVector(int index, int site, int charge)
{
    m_sHost[index] = site; m_qHost[index] = charge;
}

const double& OpenClHelper::getOutputHost(int index) const
{
    return m_oHost[index];
}

const double& OpenClHelper::getOutputHostFuture(int index) const
{
    return m_oHost[index+m_offset];
}

}
