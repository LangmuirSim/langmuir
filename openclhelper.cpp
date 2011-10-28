#include "openclhelper.h"
#include "inputparser.h"
#include "chargeagent.h"
#include "sourceagent.h"
#include "drainagent.h"
#include "potential.h"
#include "cubicgrid.h"
#include "logger.h"
#include "world.h"
#include "rand.h"

using namespace Langmuir;

OpenClHelper::OpenClHelper( World * world ) : m_world( world ) {}

void OpenClHelper::initializeOpenCL( )
{
    //can't use openCL yet
    m_world->parameters()->okCL = false;

    try
    {
        if ( ! m_world->parameters() ) qFatal("initializeOpenCL called before parameters were set.");

        cl_int err;

        //obtain platforms
        cl::vector<cl::Platform> platforms;
        err = cl::Platform::get (&platforms);
        if ( platforms.size () <= 0 ) { throw cl::Error(-1,"can not find any platforms!"); }

        //obtain devices
        cl::vector<cl::Device> devices;
        err = platforms[0].getDevices (CL_DEVICE_TYPE_GPU, &devices);
        if ( devices.size () <= 0 ) { throw cl::Error(-1,"can not find any devices!"); }

        //obtain context
        cl_context_properties contextProperties[3] =
          { CL_CONTEXT_PLATFORM, (cl_context_properties) platforms[0] (), 0 };
        m_context = cl::Context(devices, contextProperties, NULL, NULL, &err);

        //obtain command queue
        m_queue = cl::CommandQueue (m_context, devices[0], 0, &err);
        err = m_queue.finish();

        //obtain kernel source
        QDir dir(m_world->parameters()->kernelsPath);
        if (!dir.exists()) { qFatal("%s does not exist",qPrintable(dir.path()));}
        QFile file (dir.absoluteFilePath("kernel.cl"));
        if ( ! ( file.open (QIODevice::ReadOnly | QIODevice::Text) ) ) {
             throw cl::Error(-1,QString("error opening kernel file: %1").arg(dir.absoluteFilePath("kernel.cl")).toLatin1());
        }
        QByteArray contents = file.readAll ();
        file.close();
        cl::Program::Sources source (1, std::make_pair (contents, contents.size ()));
        cl::Program program(m_context, source, &err);
        err = program.build(devices, NULL, NULL, NULL);
        m_coulombK1 = cl::Kernel(program,"coulomb1",&err);
        m_coulombK2 = cl::Kernel(program,"coulomb2",&err);

        //initialize Host Memory
        m_sHost.clear( );
        m_qHost.clear( );
        m_oHost.clear( );
        m_sHost.resize( m_world->grid()->volume() );
        m_qHost.resize( m_world->grid()->volume() );
        m_oHost.resize( m_world->grid()->volume() + 2 ); // +2 for drain and source

        //initialize Device Memory
        m_sDevice = cl::Buffer(m_context, CL_MEM_READ_ONLY, m_sHost.size() * sizeof (int), NULL, &err);
        m_qDevice = cl::Buffer(m_context, CL_MEM_READ_ONLY, m_qHost.size() * sizeof (int), NULL, &err);
        m_oDevice = cl::Buffer(m_context, CL_MEM_READ_WRITE, m_oHost.size() * sizeof (double), NULL, &err);

        //upload initial data
        err = m_queue.enqueueWriteBuffer (m_sDevice, CL_TRUE, 0, m_sHost.size() * sizeof (int), &m_sHost[0], NULL, NULL);
        err = m_queue.enqueueWriteBuffer (m_sDevice, CL_TRUE, 0, m_qHost.size() * sizeof (int), &m_qHost[0], NULL, NULL);
        err = m_queue.enqueueWriteBuffer (m_oDevice, CL_TRUE, 0, m_oHost.size() * sizeof (double), &m_oHost[0], NULL, NULL);

        //preset kernel arguments that dont change
        err = m_coulombK1.setArg (0, m_oDevice);
        err = m_coulombK1.setArg (1, m_sDevice);
        err = m_coulombK1.setArg (2, m_qDevice);
        err = m_coulombK1.setArg (4, m_world->parameters()->electrostaticCutoff * m_world->parameters()->electrostaticCutoff);

        err = m_coulombK2.setArg (0, m_oDevice);
        err = m_coulombK2.setArg (1, m_sDevice);
        err = m_coulombK2.setArg (2, m_qDevice);
        err = m_coulombK2.setArg (4, m_world->parameters()->electrostaticCutoff * m_world->parameters()->electrostaticCutoff);
        err = m_coulombK2.setArg (5, m_sDevice);
        err = m_coulombK2.setArg (6, m_world->parameters()->gridWidth);
        err = m_coulombK2.setArg (7, m_world->parameters()->gridHeight);

        //check for errors
        {
            int maxCharges = m_world->grid()->volume() * ( m_world->parameters()->chargePercentage + m_world->parameters()->defectPercentage );

            //kernel1
            int wx1 = m_world->parameters()->workWidth;
            int wy1 = m_world->parameters()->workHeight;
            int wz1 = m_world->parameters()->workDepth;
            int wv1 = wx1 * wy1 * wz1;
            int gx1 = wx1 * m_world->parameters()->gridWidth;
            int gy1 = wy1 * m_world->parameters()->gridHeight;
            int gz1 = wz1 * m_world->parameters()->gridDepth;
            int gv1 = gx1 * gy1 * gz1;

            //kernel2
            int wx2 = m_world->parameters()->workSize;
            int wv2 = wx2;
            int gx2 = wx2 * maxCharges;
            int gv2 = gx2;

            //device
            cl::vector<size_t> gxyz_d = devices[0].getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>(&err);
            int gv_d = gxyz_d[0] * gxyz_d[1] * gxyz_d[2];
            int wv1_d = m_coulombK1.getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(devices[0],&err);
            int wv2_d = m_coulombK2.getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(devices[0],&err);

            QString deviceQuery = "";
            double ilog2 = 1.0 / log(2);
            int space = 12;
            deviceQuery += QString("       %1 %2 %3 : %4 %5 %6 : %7\n")
                    .arg("X",space)
                    .arg("Y",space)
                    .arg("Z",space)
                    .arg("Log2X",space)
                    .arg("Log2Y",space)
                    .arg("Log2Z",space)
                    .arg("V",space);
            deviceQuery += QString("\nKERNEL1\n");
            deviceQuery += QString("W    : %1 %2 %3 : %4 %5 %6 : %7\n")
                    .arg(wx1,space)
                    .arg(wy1,space)
                    .arg(wz1,space)
                    .arg(log(wx1)*ilog2,space)
                    .arg(log(wy1)*ilog2,space)
                    .arg(log(wz1)*ilog2,space)
                    .arg(wv1,space);
            deviceQuery += QString("G    : %1 %2 %3 : %4 %5 %6 : %7\n")
                    .arg(gx1,space)
                    .arg(gy1,space)
                    .arg(gz1,space)
                    .arg(log(gx1)*ilog2,space)
                    .arg(log(gy1)*ilog2,space)
                    .arg(log(gz1)*ilog2,space)
                    .arg(gv1,space);
            deviceQuery += QString("\nKERNEL2\n");
            deviceQuery += QString("W    : %1 %2 %3 : %4 %5 %6 : %7\n")
                    .arg(wx2,space)
                    .arg(1,space)
                    .arg(1,space)
                    .arg(log(wx2)*ilog2,space)
                    .arg(0,space)
                    .arg(0,space)
                    .arg(wv2,space);
            deviceQuery += QString("G    : %1 %2 %3 : %4 %5 %6 : %7\n")
                    .arg(gx2,space)
                    .arg(1,space)
                    .arg(1,space)
                    .arg(log(gx2)*ilog2,space)
                    .arg(0,space)
                    .arg(0,space)
                    .arg(gv2,space);
            deviceQuery += QString("\nDEVICE\n");
            deviceQuery += QString("W1   : %1\n").arg(wv1_d,space);
            deviceQuery += QString("W2   : %1\n").arg(wv2_d,space);
            deviceQuery += QString("G    : %1 %2 %3 : %4 %5 %6 : %7\n")
                    .arg(gxyz_d[0],space)
                    .arg(gxyz_d[1],space)
                    .arg(gxyz_d[2],space)
                    .arg(log(gxyz_d[0])*ilog2,space)
                    .arg(log(gxyz_d[1])*ilog2,space)
                    .arg(log(gxyz_d[2])*ilog2,space)
                    .arg(gv_d,space);
            deviceQuery += QString("\nOTHER\n");
            deviceQuery += QString("MAX  : %1\n").arg(maxCharges,space);
            deviceQuery += QString("GRID : %1 %2 %3 : %4 %5 %6 : %7\n")
                    .arg(m_world->parameters()->gridWidth,space)
                    .arg(m_world->parameters()->gridHeight,space)
                    .arg(m_world->parameters()->gridDepth,space)
                    .arg(log(m_world->parameters()->gridWidth)*ilog2,space)
                    .arg(log(m_world->parameters()->gridHeight)*ilog2,space)
                    .arg(log(m_world->parameters()->gridDepth)*ilog2,space)
                    .arg(m_world->grid()->volume(),space);

            //checks
            if ( wv2 > wv2_d )
            {
                qDebug() << qPrintable(deviceQuery);
                qFatal("work group volume of coulomb kernel 2 exceeds maxiumum for device");
            }

            if ( wv1 > wv1_d && m_world->parameters()->outputCoulombPotential )
            {
                qDebug() << qPrintable(deviceQuery);
                qFatal("work group volume of coulomb kernel 1 exceeds maxiumum for device");
            }

            if ( gv1 > gv_d && m_world->parameters()->outputCoulombPotential )
            {
                qDebug() << qPrintable(deviceQuery);
                qFatal("global work item count of coulomb kernel 1 exceeds maximum for device");
            }

            if ( gv2 > gv_d )
            {
                qDebug() << qPrintable(deviceQuery);
                qFatal("global work item count of coulomb kernel 1 exceeds maximum for device");
            }

            int bytes = ( sizeof(int) + sizeof(int) + sizeof(double) ) * maxCharges;
            int maxMemory = devices[0].getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>(&err);
            if ( bytes > maxMemory )
            {
               qWarning(" may exceed device memory - expecting crash");
            }
        }
        //force queues to finish
        err = m_queue.finish();

        //should be ok to use OpenCL
        m_world->parameters()->okCL = true;
    }
    catch(cl::Error& error)
    {
        qWarning("%s (%d)",error.what(),error.err());
        m_world->parameters()->okCL = false;
        return;
    }
    m_world->parameters()->okCL = true;
}

bool OpenClHelper::toggleOpenCL(bool on)
{
    if ( on )
    {
        if ( ! ( m_world->parameters()->okCL ) )
        {
            m_world->parameters()->useOpenCL = false;
            qDebug() << "can not use openCL on this platform - openCL has been turned off";
            return false;
        }
        if ( ! ( m_world->parameters()->interactionCoulomb ) )
        {
            m_world->parameters()->useOpenCL = false;
            qDebug() << "OpenCL is on but coulomb interactions are off - disabling openCL";
            return false;
        }
        m_world->parameters()->useOpenCL = true;
        return true;
    }
    else
    {
        m_world->parameters()->useOpenCL = false;
        return false;
    }
}

void OpenClHelper::launchCoulombKernel1( )
{
    try
    {
        int totalCharges = 0;
        //copy charges
        for ( int i = 0; i < m_world->charges()->size(); i++ )
        {
            m_sHost[i+totalCharges] = m_world->charges()->at(i)->site(false);
            m_qHost[i+totalCharges] = m_world->charges()->at(i)->charge();
            m_world->charges()->at(i)->setOpenCLID(i);
        }
        totalCharges += m_world->charges()->size();

        //copy defects
        if ( m_world->parameters()->chargedDefects )
        {
            for ( int i = 0; i < m_world->defectSiteIDs()->size(); i++ )
            {
                m_sHost[i+totalCharges] = m_world->defectSiteIDs()->at(i);
                m_qHost[i+totalCharges] = m_world->parameters()->zDefect;
            }
            totalCharges += m_world->defectSiteIDs()->size();
        }

        m_coulombK1.setArg (3, totalCharges);
        m_queue.enqueueWriteBuffer (m_sDevice,CL_TRUE,0,totalCharges*sizeof(int),&m_sHost[0]);
        m_queue.enqueueWriteBuffer (m_qDevice,CL_TRUE,0,totalCharges*sizeof(int),&m_qHost[0]);
        m_queue.enqueueNDRangeKernel(m_coulombK1,cl::NDRange (0,0,0),
          cl::NDRange (m_world->parameters()->gridWidth * m_world->parameters()->workWidth ,
                       m_world->parameters()->gridHeight* m_world->parameters()->workHeight,
                       m_world->parameters()->gridDepth * m_world->parameters()->workDepth),
          cl::NDRange (m_world->parameters()->workWidth ,
                       m_world->parameters()->workHeight,
                       m_world->parameters()->workDepth));
        m_queue.enqueueReadBuffer(m_oDevice,CL_TRUE,0,m_oHost.size()*sizeof(double),&m_oHost[0]);
        m_queue.finish();
    }
    catch(cl::Error& error)
    {
        qWarning("%s (%d)",error.what(),error.err());
        qFatal("Fatal OpenCl fatal error when calling coulomb kernel 1");
        return;
    }
}

void OpenClHelper::launchCoulombKernel2( )
{
    try
    {
        int totalCharges = 0;
        //copy charges
        for ( int i = 0; i < m_world->charges()->size(); i++ )
        {
            m_sHost[i+totalCharges] = m_world->charges()->at(i)->site(false);
            m_qHost[i+totalCharges] = m_world->charges()->at(i)->charge();
            m_world->charges()->at(i)->setOpenCLID(i);
        }
        totalCharges += m_world->charges()->size();

        //copy defects
        if ( m_world->parameters()->chargedDefects )
        {
            for ( int i = 0; i < m_world->defectSiteIDs()->size(); i++ )
            {
                m_sHost[i+totalCharges] = m_world->defectSiteIDs()->at(i);
                m_qHost[i+totalCharges] = m_world->parameters()->zDefect;
            }
            totalCharges += m_world->defectSiteIDs()->size();
        }

        //copy charges (future)
        for ( int i = 0; i < m_world->charges()->size(); i++ )
        {
            m_sHost[i+totalCharges] = m_world->charges()->at(i)->site(true);
            m_qHost[i+totalCharges] = m_world->charges()->at(i)->charge();
        }

        m_coulombK2.setArg (3, totalCharges);
        totalCharges += m_world->charges()->size();

        m_queue.enqueueWriteBuffer (m_sDevice,CL_TRUE,0,(totalCharges)*sizeof(int),&m_sHost[0]);
        m_queue.enqueueWriteBuffer (m_qDevice,CL_TRUE,0,(totalCharges)*sizeof(int),&m_qHost[0]);
        m_queue.enqueueNDRangeKernel(m_coulombK2,cl::NDRange(0),cl::NDRange(totalCharges*m_world->parameters()->workSize),cl::NDRange(m_world->parameters()->workSize));
        m_queue.enqueueReadBuffer(m_oDevice,CL_TRUE,0,m_oHost.size()*sizeof(double),&m_oHost[0]);
        m_queue.finish();
    }
    catch(cl::Error& error)
    {
        qWarning("%s (%d)",error.what(),error.err());
        qFatal("Fatal OpenCl fatal error when calling coulomb kernel 2");
        return;
    }
}

void OpenClHelper::compareHostAndDeviceForCarrier( int i )
{
    int    F  = m_world->charges()->at(i)->site(true );
    int    I  = m_world->charges()->at(i)->site(false);
    int   XI  = m_world->grid()->getColumn(I);
    int   YI  = m_world->grid()->getRow(I);
    int   ZI  = m_world->grid()->getLayer(I);
    int   XF  = m_world->grid()->getColumn(F);
    int   YF  = m_world->grid()->getRow(F);
    int   ZF  = m_world->grid()->getLayer(F);
    int    Q  = m_world->charges()->at(i)->charge();
    double H  = m_world->charges()->at(i)->interaction();
    double D  = (getOutputHost(F)-getOutputHost(I)-Q)*Q*m_world->parameters()->electrostaticPrefactor*m_world->parameters()->elementaryCharge;
    bool   C  = true;
    if ( H * D < 0 ) {
       C = (fabs(H) + fabs(D) < 1e-10);
    }
    else
    {
        C = (fabs(fabs(H)-fabs(D)) < 1e-10);
    }
    bool   d  = F >= m_world->grid()->volume();
    bool   b  = qFuzzyCompare(H,-1);
    int per   = 6;
    if ( ! b && ! d && ! C )
    {
        QString output = "";
        output += QString("H(%1) ").arg(H,12,'e',5);
        output += QString("D(%1) ").arg(D,12,'e',5);
        output += QString("C(%1) ").arg(C);
        output += QString("B(%1) ").arg(b);
        output += QString("X(%1) ").arg(d);
        output += QString("I[%1]=(%2,%3,%4) ").arg(I,per+1).arg(XI,per).arg(YI,per).arg(ZI,per);
        output += QString("F[%1]=(%2,%3,%4) ").arg(F,per+1).arg(XF,per).arg(YF,per).arg(ZF,per);
        qDebug() << output;
    }
}

void OpenClHelper::compareHostAndDeviceAtSite(int i)
{
    int XI = m_world->grid()->getColumn(i);
    int YI = m_world->grid()->getRow(i);
    int ZI = m_world->grid()->getLayer(i);
    double V = 0;
    for ( int j = 0; j < m_world->charges()->size(); j++ )
    {
        int XJ = m_world->grid()->getColumn(m_world->charges()->at(j)->site(false));
        int YJ = m_world->grid()->getRow(m_world->charges()->at(j)->site(false));
        int ZJ = m_world->grid()->getLayer(m_world->charges()->at(j)->site(false));

        double r = ( XI - XJ ) * ( XI - XJ ) + ( YI - YJ ) * ( YI - YJ ) + ( ZI - ZJ ) * ( ZI - ZJ );

        if ( r > 0 && r < m_world->parameters()->electrostaticCutoff * m_world->parameters()->electrostaticCutoff )
        {
            V = V + m_world->charges()->at(j)->charge() / sqrt(r);
        }
    }
    double D  = m_oHost[i];
    bool   C  = true;
    int per   = 6;
    if ( V * D < 0 ) {
       C = (fabs(V) + fabs(D) < 1e-10);
    }
    else
    {
        C = (fabs(fabs(V)-fabs(D)) < 1e-10);
    }
    QString output = "";
    output += QString("H(%1) ").arg(V,12,'e',5);
    output += QString("D1(%1) ").arg(D,12,'e',5);
    output += QString("C1(%1) ").arg(C);
    output += QString("I[%1]=(%2,%3,%4) ").arg(i,per+1).arg(XI,per).arg(YI,per).arg(ZI,per);
    qDebug() << output;
}

void OpenClHelper::compareHostAndDeviceForAllCarriers()
{
    for ( int j = 0; j < m_world->charges()->size(); j++ )
    {
        compareHostAndDeviceForCarrier(j);
    }
}
