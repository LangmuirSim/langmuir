#include "world.h"
#include "rand.h"
#include "inputparser.h"
#include "cubicgrid.h"

namespace Langmuir {

  World::World( int seed ) : m_grid(0), m_rand(new Rand(0.0, 1.0, seed)), m_parameters(NULL), m_statFile(NULL), m_statStream(NULL)
  {
    // This array is always the number of different sites + 2. The final two
    // rows/columns are for the source and drain site types.
    m_coupling.resize(4, 4);
    m_coupling << 0.333, 0.000, 0.333, 0.333,
                  0.000, 0.000, 0.000, 0.000,
                  0.333, 0.000, 0.002, 0.002,
                  0.333, 0.000, 0.002, 0.002;
  }

  World::~World()
  {
   delete m_rand;
   if ( m_statStream )
   {
     (*m_statStream).flush();
     delete m_statStream;
   }
   if ( m_statFile )
   {
     delete m_statFile;
   }
  }

  double World::random()
  {
   return m_rand->number();
  }

  double World::random( double average, double stdev )
  {
   return m_rand->normalNumber(average,stdev);
  }

  TripleIndexArray::TripleIndexArray()
  {
   values.clear();
   m_width = 0;
   m_area = 0;
   m_col = 0;
   m_row = 0;
   m_lay = 0;
  }

  TripleIndexArray::TripleIndexArray(int col, int row, int lay)
  {
   values.clear();
   values.resize(col*row*lay);
   m_width = col;
   m_area = col*row;
   m_col = col;
   m_row = row;
   m_lay = lay;
  }

  void TripleIndexArray::resize(int col, int row, int lay)
  {
   values.clear();
   values.resize(col*row*lay);
   m_width = col;
   m_area = col*row;
   m_col = col;
   m_row = row;
   m_lay = lay;
  }

  double& TripleIndexArray::operator() (int col, int row, int lay)
  {
   return values[col+m_width*row+lay*m_area];
  }

  double TripleIndexArray::operator() (int col, int row, int lay) const
  {
   return values[col+m_width*row+lay*m_area];
  }

  void World::initializeOpenCL( )
  {
      //can't use openCL yet
      m_parameters->okCL = false;

      try
      {
          if ( ! m_parameters ) qFatal("initializeOpenCL called before parameters were set.");

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
          QDir dir(m_parameters->kernelsPath);
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
          m_sHost.resize( m_grid->volume() );
          m_qHost.resize( m_grid->volume() );
          m_oHost.resize( m_grid->volume() + 2 ); // +2 for drain and source

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
          err = m_coulombK1.setArg (4, m_parameters->electrostaticCutoff * m_parameters->electrostaticCutoff);

          err = m_coulombK2.setArg (0, m_oDevice);
          err = m_coulombK2.setArg (1, m_sDevice);
          err = m_coulombK2.setArg (2, m_qDevice);
          err = m_coulombK2.setArg (4, m_parameters->electrostaticCutoff * m_parameters->electrostaticCutoff);
          err = m_coulombK2.setArg (5, m_sDevice);
          err = m_coulombK2.setArg (6, m_parameters->gridWidth);
          err = m_coulombK2.setArg (7, m_parameters->gridHeight);

          //check for errors
          {
              int maxCharges = m_grid->volume() * ( m_parameters->chargePercentage + m_parameters->defectPercentage );

              //kernel1
              int wx1 = m_parameters->workWidth;
              int wy1 = m_parameters->workHeight;
              int wz1 = m_parameters->workDepth;
              int wv1 = wx1 * wy1 * wz1;
              int gx1 = wx1 * m_parameters->gridWidth;
              int gy1 = wy1 * m_parameters->gridHeight;
              int gz1 = wz1 * m_parameters->gridDepth;
              int gv1 = gx1 * gy1 * gz1;

              //kernel2
              int wx2 = m_parameters->workSize;
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
                      .arg(m_parameters->gridWidth,space)
                      .arg(m_parameters->gridHeight,space)
                      .arg(m_parameters->gridDepth,space)
                      .arg(log(m_parameters->gridWidth)*ilog2,space)
                      .arg(log(m_parameters->gridHeight)*ilog2,space)
                      .arg(log(m_parameters->gridDepth)*ilog2,space)
                      .arg(m_grid->volume(),space);

              //checks
              if ( wv2 > wv2_d )
              {
                  qDebug() << qPrintable(deviceQuery);
                  qFatal("work group volume of coulomb kernel 2 exceeds maxiumum for device");
              }

              if ( wv1 > wv1_d && m_parameters->outputCoulombPotential )
              {
                  qDebug() << qPrintable(deviceQuery);
                  qFatal("work group volume of coulomb kernel 1 exceeds maxiumum for device");
              }

              if ( gv1 > gv_d && m_parameters->outputCoulombPotential )
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
          m_parameters->okCL = true;
      }
      catch(cl::Error& error)
      {
          qWarning("%s (%d)",error.what(),error.err());
          m_parameters->okCL = false;
          return;
      }
      m_parameters->okCL = true;
  }

  bool World::toggleOpenCL(bool on)
  {
      if ( on )
      {
          if ( ! ( m_parameters->okCL ) )
          {
              m_parameters->useOpenCL = false;
              qDebug() << "can not use openCL on this platform - openCL has been turned off";
              return false;
          }
          if ( ! ( m_parameters->interactionCoulomb ) )
          {
              m_parameters->useOpenCL = false;
              qDebug() << "OpenCL is on but coulomb interactions are off - disabling openCL";
              return false;
          }
          m_parameters->useOpenCL = true;
          return true;
      }
      else
      {
          m_parameters->useOpenCL = false;
          return false;
      }
  }

  void World::launchCoulombKernel1( )
  {
      try
      {
          int totalCharges = 0;
          //copy charges
          for ( int i = 0; i < m_charges.size(); i++ )
          {
              m_sHost[i+totalCharges] = m_charges[i]->site(false);
              m_qHost[i+totalCharges] = m_charges[i]->charge();
              m_charges[i]->setOpenCLID(i);
          }
          totalCharges += m_charges.size();

          //copy defects
          if ( m_parameters->chargedDefects )
          {
              for ( int i = 0; i < m_defectSiteIDs.size(); i++ )
              {
                  m_sHost[i+totalCharges] = m_defectSiteIDs[i];
                  m_qHost[i+totalCharges] = m_parameters->zDefect;
              }
              totalCharges += m_defectSiteIDs.size();
          }

          m_coulombK1.setArg (3, totalCharges);
          m_queue.enqueueWriteBuffer (m_sDevice,CL_TRUE,0,totalCharges*sizeof(int),&m_sHost[0]);
          m_queue.enqueueWriteBuffer (m_qDevice,CL_TRUE,0,totalCharges*sizeof(int),&m_qHost[0]);
          m_queue.enqueueNDRangeKernel(m_coulombK1,cl::NDRange (0,0,0),
            cl::NDRange (m_parameters->gridWidth * m_parameters->workWidth ,
                         m_parameters->gridHeight* m_parameters->workHeight,
                         m_parameters->gridDepth * m_parameters->workDepth),
            cl::NDRange (m_parameters->workWidth ,
                         m_parameters->workHeight,
                         m_parameters->workDepth));
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

  void World::launchCoulombKernel2( )
  {
      try
      {
          int totalCharges = 0;
          //copy charges
          for ( int i = 0; i < m_charges.size(); i++ )
          {
              m_sHost[i+totalCharges] = m_charges[i]->site(false);
              m_qHost[i+totalCharges] = m_charges[i]->charge();
              m_charges[i]->setOpenCLID(i);
          }
          totalCharges += m_charges.size();

          //copy defects
          if ( m_parameters->chargedDefects )
          {
              for ( int i = 0; i < m_defectSiteIDs.size(); i++ )
              {
                  m_sHost[i+totalCharges] = m_defectSiteIDs[i];
                  m_qHost[i+totalCharges] = m_parameters->zDefect;
              }
              totalCharges += m_defectSiteIDs.size();
          }

          //copy charges (future)
          for ( int i = 0; i < m_charges.size(); i++ )
          {
              m_sHost[i+totalCharges] = m_charges[i]->site(true);
              m_qHost[i+totalCharges] = m_charges[i]->charge();
          }

          m_coulombK2.setArg (3, totalCharges);
          totalCharges += m_charges.size();

          m_queue.enqueueWriteBuffer (m_sDevice,CL_TRUE,0,(totalCharges)*sizeof(int),&m_sHost[0]);
          m_queue.enqueueWriteBuffer (m_qDevice,CL_TRUE,0,(totalCharges)*sizeof(int),&m_qHost[0]);
          m_queue.enqueueNDRangeKernel(m_coulombK2,cl::NDRange(0),cl::NDRange(totalCharges*m_parameters->workSize),cl::NDRange(m_parameters->workSize));
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

  void World::compareHostAndDeviceForCarrier( int i )
  {
      int    F  = m_charges[i]->site(true );
      int    I  = m_charges[i]->site(false);
      int   XI  = m_grid->getColumn(I);
      int   YI  = m_grid->getRow(I);
      int   ZI  = m_grid->getLayer(I);
      int   XF  = m_grid->getColumn(F);
      int   YF  = m_grid->getRow(F);
      int   ZF  = m_grid->getLayer(F);
      int    Q  = m_charges[i]->charge();
      double H  = m_charges[i]->interaction();
      double D  = (getOutputHost(F)-getOutputHost(I)-Q)*Q*m_parameters->electrostaticPrefactor*m_parameters->elementaryCharge;
      bool   C  = true;
      if ( H * D < 0 ) {
         C = (fabs(H) + fabs(D) < 1e-10);
      }
      else
      {
          C = (fabs(fabs(H)-fabs(D)) < 1e-10);
      }
      bool   d  = F >= m_grid->volume();
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

  void World::compareHostAndDeviceAtSite(int i)
  {
      int XI = m_grid->getColumn(i);
      int YI = m_grid->getRow(i);
      int ZI = m_grid->getLayer(i);
      double V = 0;
      for ( int j = 0; j < m_charges.size(); j++ )
      {
          int XJ = m_grid->getColumn(m_charges[j]->site(false));
          int YJ = m_grid->getRow(m_charges[j]->site(false));
          int ZJ = m_grid->getLayer(m_charges[j]->site(false));

          double r = ( XI - XJ ) * ( XI - XJ ) + ( YI - YJ ) * ( YI - YJ ) + ( ZI - ZJ ) * ( ZI - ZJ );

          if ( r > 0 && r < m_parameters->electrostaticCutoff * m_parameters->electrostaticCutoff )
          {
              V = V + m_charges[j]->charge() / sqrt(r);
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

  void World::compareHostAndDeviceForAllCarriers()
  {
      for ( int j = 0; j < m_charges.size(); j++ )
      {
          compareHostAndDeviceForCarrier(j);
      }
  }

  void World::saveCarrierIDsToFile( QString name )
  {
      QFile out(name);
      int w = m_parameters->outputWidth;
      if ( !(out.open(QIODevice::WriteOnly | QIODevice::Text)) )
      {
          qFatal("can not open file %s",qPrintable(name));
      }
      for ( int i = 0; i < m_charges.size(); i++ )
      {
          int s = m_charges[i]->site(false);
          int x = m_grid->getColumn(s);
          int y = m_grid->getRow(s);
          int z = m_grid->getLayer(s);
          out.write( qPrintable( QString("%1 %2 %3 %4\n").arg(x,w).arg(y,w).arg(z,w).arg(s,w) ) );
      }
  }

  void World::saveTrapIDsToFile( QString name )
  {
      if ( m_trapSiteIDs.size() == 0 ) return;
      QFile out(name);
      int w = m_parameters->outputWidth;
      if ( !(out.open(QIODevice::WriteOnly | QIODevice::Text)) )
      {
          qFatal("can not open file %s",qPrintable(name));
      }
      for ( int i = 0; i < m_trapSiteIDs.size(); i++ )
      {
          int s = m_trapSiteIDs[i];
          int x = m_grid->getColumn(s);
          int y = m_grid->getRow(s);
          int z = m_grid->getLayer(s);
          out.write( qPrintable( QString("%1 %2 %3 %4\n").arg(x,w).arg(y,w).arg(z,w).arg(s,w) ) );
      }
      out.close();
  }

  void World::saveDefectIDsToFile( QString name )
  {
      if ( m_defectSiteIDs.size() == 0 ) return;
      QFile out(name);
      int w = m_parameters->outputWidth;
      if ( !(out.open(QIODevice::WriteOnly | QIODevice::Text)) )
      {
          qFatal("can not open file %s",qPrintable(name));
      }
      for ( int i = 0; i < m_defectSiteIDs.size(); i++ )
      {
          int s = m_defectSiteIDs[i];
          int x = m_grid->getColumn(s);
          int y = m_grid->getRow(s);
          int z = m_grid->getLayer(s);
          out.write( qPrintable( QString("%1 %2 %3 %4\n").arg(x,w).arg(y,w).arg(z,w).arg(s,w) ) );
      }
      out.close();
  }

  void World::saveFieldEnergyToFile( QString name )
  {
      QFile out(name);
      int p = m_parameters->outputPrecision;
      int w = m_parameters->outputWidth;
      double te = 0;
      if ( m_trapSiteIDs.size() > 0 ) { te = m_parameters->deltaEpsilon; }
      if ( !(out.open(QIODevice::WriteOnly | QIODevice::Text)) )
      {
          qFatal("can not open file %s",qPrintable(name));
      }
      for ( int i = 0; i < m_grid->width(); i++ )
      {
          for ( int j = 0; j < m_grid->height(); j++ )
          {
              for ( int k = 0; k < m_grid->depth(); k++ )
              {
                  int s = m_grid->getIndex(i,j,k);
                  if ( m_trapSiteIDs.contains(s) )
                  {
                      out.write( qPrintable( QString("%1 %2 %3 %4\n").arg(i,w).arg(j,w).arg(k,w).arg(m_grid->potential(s)-te,w,'e',p) ) );
                  }
                  else
                  {
                      out.write( qPrintable( QString("%1 %2 %3 %4\n").arg(i,w).arg(j,w).arg(k,w).arg(m_grid->potential(s),w,'e',p) ) );
                  }
              }
          }
      }
      out.close();
  }

  void World::saveTrapEnergyToFile( QString name )
  {
      if ( m_trapSiteIDs.size() == 0 ) return;
      QFile out(name);
      int p = m_parameters->outputPrecision;
      int w = m_parameters->outputWidth;
      if ( !(out.open(QIODevice::WriteOnly | QIODevice::Text)) )
      {
          qFatal("can not open file %s",qPrintable(name));
      }
      for ( int i = 0; i < m_grid->width(); i++ )
      {
          for ( int j = 0; j < m_grid->height(); j++ )
          {
              for ( int k = 0; k < m_grid->depth(); k++ )
              {
                  int s = m_grid->getIndex(i,j,k);
                  if ( m_trapSiteIDs.contains(s) )
                  {
                      out.write( qPrintable( QString("%1 %2 %3 %4\n").arg(i,w).arg(j,w).arg(k,w).arg(m_parameters->deltaEpsilon,w,'e',p) ) );
                  }
                  else
                  {
                      out.write( qPrintable( QString("%1 %2 %3 %4\n").arg(i,w).arg(j,w).arg(k,w).arg(0.0,w,'e',p) ) );
                  }
              }
          }
      }
      out.close();
  }

  void World::saveCoulombEnergyToFile( QString name )
  {
      QFile out(name);
      int p = m_parameters->outputPrecision;
      int w = m_parameters->outputWidth;
      if ( !(out.open(QIODevice::WriteOnly | QIODevice::Text)) )
      {
          qFatal("can not open file %s",qPrintable(name));
      }
      for ( int i = 0; i < m_grid->width(); i++ )
      {
          for ( int j = 0; j < m_grid->height(); j++ )
          {
              for ( int k = 0; k < m_grid->depth(); k++ )
              {
                  int s = m_grid->getIndex(i,j,k);
                  out.write( qPrintable( QString("%1 %2 %3 %4\n").arg(i,w).arg(j,w).arg(k,w).arg(m_oHost[s]*m_parameters->electrostaticPrefactor,w,'e',p) ) );
              }
          }
      }
      out.close();
  }

  void World::setParameters(SimulationParameters *parameters)
  {
      m_parameters = parameters;

      if (m_parameters->outputStats)
        {
          m_statFile = new QFile ("stats");
          if (!(m_statFile->open (QIODevice::WriteOnly | QIODevice::Text)))
            {
              qDebug () << "Error opening stat file:";
            }
          m_statStream = new QTextStream (m_statFile);
          m_statStream->setRealNumberPrecision (m_parameters->outputPrecision);
          m_statStream->setFieldWidth (m_parameters->outputWidth);
        *(m_statStream) << scientific;
        }
  }

  void World::statMessage( QString message )
  {
      (*m_statStream) << qPrintable( message );
  }

  void World::statFlush( )
  {
      (*m_statStream).flush();
  }

}
