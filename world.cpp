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

  TripleIndexArray::TripleIndexArray(unsigned int col, unsigned int row, unsigned int lay)
  {
   values.clear();
   values.resize(col*row*lay);
   m_width = col;
   m_area = col*row;
   m_col = col;
   m_row = row;
   m_lay = lay;
  }

  void TripleIndexArray::resize(unsigned int col, unsigned int row, unsigned int lay)
  {
   values.clear();
   values.resize(col*row*lay);
   m_width = col;
   m_area = col*row;
   m_col = col;
   m_row = row;
   m_lay = lay;
  }

  double& TripleIndexArray::operator() (unsigned int col, unsigned int row, unsigned int lay)
  {
   return values[col+m_width*row+lay*m_area];
  }

  double TripleIndexArray::operator() (unsigned int col, unsigned int row, unsigned int lay) const
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

          //obtain platforms
          m_error = cl::Platform::get (&m_platforms);
          if ( m_platforms.size () <= 0 ) { throw cl::Error(-1,"can not find any platforms!"); }

          //obtain devices
          m_error = m_platforms[0].getDevices (CL_DEVICE_TYPE_GPU, &m_devices);
          if ( m_devices.size () <= 0 ) { throw cl::Error(-1,"can not find any devices!"); }

          //obtain context
          cl_context_properties contextProperties[3] =
            { CL_CONTEXT_PLATFORM, (cl_context_properties) m_platforms[0] (), 0 };
          m_contexts.push_back (cl::Context(m_devices, contextProperties, NULL, NULL, &m_error));

          //obtain command queue
          m_queues.push_back (cl::CommandQueue (m_contexts[0], m_devices[0], 0, &m_error));
          m_error = m_queues[0].finish();

          //obtain main kernel source
          QDir dir(m_parameters->kernelsPath);
          QFile file (dir.absoluteFilePath("kernel.cl"));
          if ( ! ( file.open (QIODevice::ReadOnly | QIODevice::Text) ) ) {
               throw cl::Error(-1,QString("m_error opening kernel file: %1").arg(dir.absoluteFilePath("kernel.cl")).toLatin1());
          }

          QByteArray contents = file.readAll ();
          QString define = QString("#define PREFACTOR %1\n#define CUTOFF2 %2\n#define ASIZE %3\n#define XSIZE %4\n#define YSIZE %5\n#define NCHARGES 10\n").
           arg(m_parameters->electrostaticPrefactor * m_parameters->elementaryCharge,0,'e',100).
           arg(m_parameters->electrostaticCutoff*m_parameters->electrostaticCutoff).
           arg(m_parameters->gridWidth*m_parameters->gridHeight).
           arg(m_parameters->gridWidth).
           arg(m_parameters->gridHeight);
          contents.insert (0, define);
          file.close();

          //make the program string into the source
          cl::Program::Sources source (1, std::make_pair (contents, contents.size ()));

          //make the source into a program associated with the context
          m_programs.push_back (cl::Program (m_contexts[0], source, &m_error));

          //compile the program
          m_error = m_programs[0].build (m_devices, NULL, NULL, NULL);

          //construct the kernel
          m_kernels.push_back( cl::Kernel(m_programs[0],"popcorn",&m_error) );

          //obtain defect kernel source
          QFile file2 (dir.absoluteFilePath("defects.cl"));
          if ( ! ( file2.open (QIODevice::ReadOnly | QIODevice::Text) ) ) {
              throw cl::Error(-1,QString("m_error opening defects kernel file: %1").arg(dir.absoluteFilePath("defects.cl")).toLatin1());
          }
          contents.clear();
          contents = file2.readAll ();
          contents.insert (0, define);
          file2.close();

          //make the program string into the source
          source = cl::Program::Sources(1, std::make_pair (contents, contents.size ()));

          //make the source into a program associated with the context
          m_programs.push_back (cl::Program (m_contexts[0], source, &m_error));

          //compile the program
          m_error = m_programs[1].build (m_devices, NULL, NULL, NULL);

          //construct the kernel
          m_kernels.push_back( cl::Kernel(m_programs[1],"defects",&m_error) );

          //figure out global work size
          int maxCharges = m_parameters->chargePercentage * double( m_grid->volume() );
          m_parameters->globalSize = maxCharges * m_parameters->workSize;

          //check for errors - with max charges
          if ( maxCharges == 0 ) {
              throw cl::Error(-1,"can not use OpenCL when there are no charges!");
          }

          //check for errors - work size
          if ( m_parameters->workSize == 0 ) {
              throw cl::Error(-1,"can not use OpenCL with zero work size! - try a power of two (ex: 32)");
          }
          int maxWorkGroupSize = m_devices[0].getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>(&m_error);
          if ( m_parameters->workSize > maxWorkGroupSize ) {
              throw cl::Error(-1,QString("local work size can not exceed %1!").arg(maxWorkGroupSize).toLatin1());
          }
          if ( (m_parameters->workSize & (m_parameters->workSize-1))) {
              throw cl::Error(-1,"local work size should be a power of 2!");
          }

          //check for errors - global size
          if ( m_parameters->globalSize == 0 ) {
              throw cl::Error(-1,"global work item size was set to 0!");
          }
          std::vector<size_t> maxWorkItemSizes = m_devices[0].getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>(&m_error);
          if ( m_parameters->globalSize > int(maxWorkItemSizes[0]*maxWorkItemSizes[1]*maxWorkItemSizes[2]) )
          {
           throw cl::Error(-1,QString("global work item size exceeds max = %1*%2*%3 = %4,  try a smaller work size or lower charge.percentage")
            .arg(int(maxWorkItemSizes[0]))
            .arg(int(maxWorkItemSizes[1]))
            .arg(int(maxWorkItemSizes[0]))
            .arg(int(maxWorkItemSizes[2]*maxWorkItemSizes[1]*maxWorkItemSizes[2])).toLatin1());
          }

          //check for errors - global size memory usage
          int bytes = ( sizeof(int) + sizeof(int) + sizeof(double) ) * maxCharges;
          int maxMemory = m_devices[0].getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>(&m_error);
          if ( bytes > maxMemory )
          {
           qWarning("max charges may exceed device memory - expecting crash");
          }

          //initialize Host Memory
          m_iHost.resize( maxCharges, 0 );
          m_fHost.resize( maxCharges, 0 );
          m_oHost.resize( maxCharges, 0 );

          //initialize Device Memory
          m_iDevice = cl::Buffer(m_contexts[0], CL_MEM_READ_ONLY, maxCharges * sizeof (int), NULL, &m_error);
          m_fDevice = cl::Buffer(m_contexts[0], CL_MEM_READ_ONLY, maxCharges * sizeof (int), NULL, &m_error);
          m_oDevice = cl::Buffer(m_contexts[0], CL_MEM_WRITE_ONLY, maxCharges * sizeof (double), NULL, &m_error);

          //upload initial data
          m_error = m_queues[0].enqueueWriteBuffer (m_fDevice, CL_TRUE, 0, m_fHost.size() * sizeof (int), &m_fHost[0], NULL, NULL);
          m_error = m_queues[0].enqueueWriteBuffer (m_iDevice, CL_TRUE, 0, m_iHost.size() * sizeof (int), &m_iHost[0], NULL, NULL);
          m_error = m_queues[0].enqueueWriteBuffer (m_oDevice, CL_TRUE, 0, m_oHost.size() * sizeof (float), &m_oHost[0], NULL, NULL);

          //prepare for charged defects
          if ( m_parameters->defectPercentage > 0 && m_defectSiteIDs.size() > 0 )
          {
              //initialize device memory
              m_dDevice = cl::Buffer(m_contexts[0], CL_MEM_READ_ONLY, m_defectSiteIDs.size() * sizeof (int), NULL, &m_error);
              //create continuous memory on host with defect site ids
              QVector<int> dHost = QVector<int>( m_defectSiteIDs.size() );
              for ( int i = 0; i < m_defectSiteIDs.size(); i++ )
              {
                  dHost[i] = m_defectSiteIDs[i];
              }
              //upload defect site ids
              m_error = m_queues[0].enqueueWriteBuffer (m_dDevice, CL_TRUE, 0, m_defectSiteIDs.size() * sizeof (int), &dHost[0], NULL, NULL);
          }

          //preset kernel arguments that dont change
          m_error = m_kernels[0].setArg (0, m_iDevice);
          m_error = m_kernels[0].setArg (1, m_fDevice);
          m_error = m_kernels[0].setArg (2, m_oDevice);
          m_error = m_kernels[1].setArg (0, m_iDevice);
          m_error = m_kernels[1].setArg (1, m_fDevice);
          m_error = m_kernels[1].setArg (2, m_dDevice);
          m_error = m_kernels[1].setArg (3, m_oDevice);
          m_error = m_kernels[1].setArg (4, m_defectSiteIDs.size() );
          m_error = m_kernels[1].setArg (5, m_parameters->zDefect );

          //force queues to finish
          m_error = m_queues[0].finish();

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

  const bool World::canUseOpenCL() const
  {
   return m_parameters->okCL;
  }

  void World::launchKernel( )
  {
      try
      {
          m_error = m_kernels[0].setArg (3, m_charges.size() );
          m_error = m_queues[0].enqueueNDRangeKernel (m_kernels[0], cl::NDRange (0), cl::NDRange (m_parameters->globalSize), cl::NDRange (m_parameters->workSize), NULL, NULL);
          if ( m_parameters->chargedDefects )
          {
            m_error = m_queues[0].enqueueNDRangeKernel (m_kernels[1], cl::NDRange (0), cl::NDRange (m_parameters->globalSize), cl::NDRange (m_parameters->workSize), NULL, NULL);
          }
      }
      catch(cl::Error& error)
      {
          qWarning("%s (%d)",error.what(),error.err());
          qFatal("Fatal OpenCl fatal error when calling kernels");
          return;
      }
  }

  void World::copyDeviceToHost( )
  {
      try
      {
          m_error = m_queues[0].enqueueReadBuffer (m_oDevice, CL_TRUE, 0, m_charges.size() * sizeof (double), &m_oHost[0], NULL, NULL);
      }
      catch(cl::Error& error)
      {
          qWarning("%s (%d)",error.what(),error.err());
          qFatal("Fatal OpenCl fatal error when copying from device to host");
          return;
      }
  }

  void World::copyHostToDevice( )
  {
      try
      {
          m_error = m_queues[0].enqueueWriteBuffer (m_iDevice, CL_TRUE, 0, m_charges.size() * sizeof (int), &m_iHost[0], NULL, NULL);
          m_error = m_queues[0].enqueueWriteBuffer (m_fDevice, CL_TRUE, 0, m_charges.size() * sizeof (int), &m_fHost[0], NULL, NULL);
      }
      catch(cl::Error& error)
      {
          qWarning("%s (%d)",error.what(),error.err());
          qFatal("Fatal OpenCl fatal error when copying from host to device");
          return;
      }
  }

  void World::clFinish()
  {
      try
      {
              m_error = m_queues[0].finish();
      }
      catch(cl::Error& error)
      {
          qWarning("%s (%d)",error.what(),error.err());
          qFatal("Fatal OpenCl fatal error at OpenCl finish barrier");
          return;
      }
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
