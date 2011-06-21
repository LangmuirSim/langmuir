#include "world.h"
#include "rand.h"
#include "inputparser.h"
#include "cubicgrid.h"

namespace Langmuir {

  World::World( int seed ) : m_grid(0), m_rand(new Rand(0.0, 1.0, seed)), m_parameters(NULL)
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
    if ( ! m_parameters ) qFatal("initializeOpenCL called before parameters were set.");

    //can't use openCL yet
    m_parameters->okCL = false;

    //obtain platforms
    m_error = cl::Platform::get (&m_platforms);
    if ( m_platforms.size () <= 0 ) {
     qDebug() << "can not find any platforms!";
     return; }
    if ( m_error != CL_SUCCESS ) {
     qDebug() << qPrintable(clErrorString (m_error));
     return; }

    //obtain devices
    m_error = m_platforms[0].getDevices (CL_DEVICE_TYPE_GPU, &m_devices);
    if ( m_devices.size () <= 0 ) {
     qDebug() << "can not find any devices!";
     return; }
    if ( m_error != CL_SUCCESS ) {
     qDebug() << qPrintable(clErrorString (m_error));
     return; }

    //obtain context
    cl_context_properties contextProperties[3] =
      { CL_CONTEXT_PLATFORM, (cl_context_properties) m_platforms[0] (), 0 };
    m_contexts.push_back (cl::Context(m_devices, contextProperties, NULL, NULL, &m_error));
    if ( m_error != CL_SUCCESS ) {
     qDebug() << qPrintable(clErrorString (m_error));
     return; }

    //obtain command queue
    m_queues.push_back (cl::CommandQueue (m_contexts[0], m_devices[0], 0, &m_error));
    if ( m_error != CL_SUCCESS ) {
     qDebug() << qPrintable(clErrorString (m_error));
     return; }
    m_error = m_queues[0].finish();
    if ( m_error != CL_SUCCESS ) {
     qDebug() << qPrintable(clErrorString (m_error));
     return; }

    //obtain main kernel source
    QFile file (m_parameters->kernelFile);
    if ( ! ( file.open (QIODevice::ReadOnly | QIODevice::Text) ) ) {
     qDebug() << qPrintable(QString("m_error opening kernel file: %1").arg(m_parameters->kernelFile));
     return; }
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
    if ( m_error != CL_SUCCESS ) {
     qDebug() << qPrintable(clErrorString(m_error));
     return; }

    //compile the program
    m_error = m_programs[0].build (m_devices, NULL, NULL, NULL);
    if ( m_error != CL_SUCCESS ) {
     qDebug() << qPrintable(clErrorString(m_error));
     return; }

    //construct the kernel
    m_kernels.push_back( cl::Kernel(m_programs[0],"popcorn",&m_error) );
    if ( m_error != CL_SUCCESS ) {
     qDebug() << qPrintable(clErrorString(m_error));
     return; }

    //obtain defect kernel source
    QFile file2 ("defects.cl");
    if ( ! ( file2.open (QIODevice::ReadOnly | QIODevice::Text) ) ) {
     qDebug() << qPrintable(QString("m_error opening kernel file: %1").arg("defects.cl"));
     return; }
    contents.clear();
    contents = file2.readAll ();
    contents.insert (0, define);
    file2.close();

    //make the program string into the source
    source = cl::Program::Sources(1, std::make_pair (contents, contents.size ()));

    //make the source into a program associated with the context
    m_programs.push_back (cl::Program (m_contexts[0], source, &m_error));
    if ( m_error != CL_SUCCESS ) {
     qDebug() << qPrintable(clErrorString(m_error));
     return; }

    //compile the program
    m_error = m_programs[1].build (m_devices, NULL, NULL, NULL);
    if ( m_error != CL_SUCCESS ) {
     qDebug() << qPrintable(clErrorString(m_error));
     return; }

    //construct the kernel
    m_kernels.push_back( cl::Kernel(m_programs[1],"defects",&m_error) );
    if ( m_error != CL_SUCCESS ) {
     qDebug() << qPrintable(clErrorString(m_error));
     return; }

    //figure out global work size
    int maxCharges = m_parameters->chargePercentage * double( m_grid->volume() );
    m_parameters->globalSize = maxCharges * m_parameters->workSize;

    //check for errors - with max charges
    if ( maxCharges == 0 ) qFatal("can not use OpenCL when there are no charges");

    //check for errors - work size
    if ( m_parameters->workSize == 0 ) qFatal("can not use OpenCL with zero work size - try a power of two (ex: 32)");
    int maxWorkGroupSize =
     m_devices[0].getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>(&m_error);
    if ( m_parameters->workSize > maxWorkGroupSize )
     qFatal("local work size can not exceed %d",maxWorkGroupSize);
    if ( (m_parameters->workSize & (m_parameters->workSize-1)))
     qFatal("local work size should be a power of 2");

    //check for errors - global size
    if ( m_parameters->globalSize == 0 ) qFatal("global work item size was set to 0");
    std::vector<size_t> maxWorkItemSizes =
     m_devices[0].getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>(&m_error);
    if ( m_parameters->globalSize > int(maxWorkItemSizes[0]*maxWorkItemSizes[1]*maxWorkItemSizes[2]) )
     qFatal("global work item size exceeds max = %d*%d*%d = %d,  try a smaller work size or lower charge.percentage",
      int(maxWorkItemSizes[0]),
      int(maxWorkItemSizes[1]),
      int(maxWorkItemSizes[0]),
      int(maxWorkItemSizes[2]*
          maxWorkItemSizes[1]*
          maxWorkItemSizes[2]));

    //check for errors - global size memory usage
    int bytes = ( sizeof(int) + sizeof(int) + sizeof(double) ) * maxCharges;
    int maxMemory = m_devices[0].getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>(&m_error);
    if ( bytes > maxMemory )
     qWarning("max charges may exceed device memory - expecting crash");

    //initialize Host Memory
    m_iHost.resize( maxCharges, 0 );
    m_fHost.resize( maxCharges, 0 );
    m_oHost.resize( maxCharges, 0 );

    //initialize Device Memory
    m_iDevice = cl::Buffer(m_contexts[0], CL_MEM_READ_ONLY, maxCharges * sizeof (int), NULL, &m_error);
    if ( m_error != CL_SUCCESS ) {
     qDebug() << qPrintable (clErrorString (m_error));
     return; }

    m_fDevice = cl::Buffer(m_contexts[0], CL_MEM_READ_ONLY, maxCharges * sizeof (int), NULL, &m_error);
    if ( m_error != CL_SUCCESS ) {
     qDebug() << qPrintable (clErrorString (m_error));
     return; }

    m_oDevice = cl::Buffer(m_contexts[0], CL_MEM_WRITE_ONLY, maxCharges * sizeof (double), NULL, &m_error);
    if ( m_error != CL_SUCCESS ) {
     qDebug() << qPrintable (clErrorString (m_error));
     return; }

    //upload initial data
    m_error = m_queues[0].enqueueWriteBuffer (m_fDevice, CL_TRUE, 0, m_charges.size() * sizeof (int), &m_fHost[0], NULL, NULL);
    if ( m_error != CL_SUCCESS ) {
     qDebug() << qPrintable (clErrorString (m_error));
     return; }

    m_error = m_queues[0].enqueueWriteBuffer (m_iDevice, CL_TRUE, 0, m_charges.size() * sizeof (int), &m_iHost[0], NULL, NULL);
    if ( m_error != CL_SUCCESS ) {
     qDebug() << qPrintable (clErrorString (m_error));
     return; }

    m_error = m_queues[0].enqueueWriteBuffer (m_oDevice, CL_TRUE, 0, m_charges.size() * sizeof (float), &m_oHost[0], NULL, NULL);
    if ( m_error != CL_SUCCESS ) {
     qDebug() << qPrintable (clErrorString (m_error));
     return; }

    //prepare for charged defects
    if ( m_parameters->defectPercentage > 0 && m_defectSiteIDs.size() > 0 )
    {
        //initialize device memory
        m_dDevice = cl::Buffer(m_contexts[0], CL_MEM_READ_ONLY, m_defectSiteIDs.size() * sizeof (int), NULL, &m_error);
        if ( m_error != CL_SUCCESS ) {
         qDebug() << qPrintable (clErrorString (m_error));
         return; }

        //create continuous memory on host with defect site ids
        QVector<int> dHost = QVector<int>( m_defectSiteIDs.size() );
        for ( int i = 0; i < m_defectSiteIDs.size(); i++ )
        {
            dHost[i] = m_defectSiteIDs[i];
        }

        //upload defect site ids
        m_error = m_queues[0].enqueueWriteBuffer (m_dDevice, CL_TRUE, 0, m_defectSiteIDs.size() * sizeof (int), &dHost[0], NULL, NULL);
        if ( m_error != CL_SUCCESS ) {
         qDebug() << qPrintable (clErrorString (m_error));
         return; }
    }

    //preset kernel arguments that dont change
    m_error = m_kernels[0].setArg (0, m_iDevice);
    if ( m_error != CL_SUCCESS ) {
     qDebug() << qPrintable (clErrorString (m_error));
     return; }

    m_error = m_kernels[0].setArg (1, m_fDevice);
    if ( m_error != CL_SUCCESS ) {
     qDebug() << qPrintable (clErrorString (m_error));
     return; }

    m_error = m_kernels[0].setArg (2, m_oDevice);
    if ( m_error != CL_SUCCESS ) {
     qDebug() << qPrintable (clErrorString (m_error));
     return; }

    m_error = m_kernels[1].setArg (0, m_iDevice);
    if ( m_error != CL_SUCCESS ) {
     qDebug() << qPrintable (clErrorString (m_error));
     return; }

    m_error = m_kernels[1].setArg (1, m_fDevice);
    if ( m_error != CL_SUCCESS ) {
     qDebug() << qPrintable (clErrorString (m_error));
     return; }
    m_error = m_kernels[1].setArg (2, m_dDevice);
    if ( m_error != CL_SUCCESS ) {
     qDebug() << qPrintable (clErrorString (m_error));
     return; }

    m_error = m_kernels[1].setArg (3, m_oDevice);
    if ( m_error != CL_SUCCESS ) {
     qDebug() << qPrintable (clErrorString (m_error));
     return; }

    m_error = m_kernels[1].setArg (4, m_defectSiteIDs.size() );
    if ( m_error != CL_SUCCESS ) {
     qDebug() << qPrintable (clErrorString (m_error));
     return; }

    m_error = m_kernels[1].setArg (5, m_parameters->zDefect );
    if ( m_error != CL_SUCCESS ) {
     qDebug() << qPrintable (clErrorString (m_error));
     return; }

    //force queues to finish
    m_error = m_queues[0].finish();
    if ( m_error != CL_SUCCESS ) {
     qDebug() << qPrintable (clErrorString (m_error));
     return; }

    //should be ok to use OpenCL
    m_parameters->okCL = true;
  }

  const bool World::canUseOpenCL() const
  {
   return m_parameters->okCL;
  }

  void World::launchKernel( )
  {
    m_error = m_kernels[0].setArg (3, m_charges.size() );
    Q_ASSERT_X (m_error == CL_SUCCESS, "cl::Kernel::setArg", qPrintable (clErrorString (m_error)));

    m_error = m_queues[0].enqueueNDRangeKernel (m_kernels[0], cl::NDRange (0), cl::NDRange (m_parameters->globalSize), cl::NDRange (m_parameters->workSize), NULL, NULL);
    Q_ASSERT_X (m_error == CL_SUCCESS, "cl::CommandQueue::enqueueNDRangeKernel", qPrintable (clErrorString (m_error)));

    if ( m_parameters->chargedDefects )
    {
      m_error = m_queues[0].enqueueNDRangeKernel (m_kernels[1], cl::NDRange (0), cl::NDRange (m_parameters->globalSize), cl::NDRange (m_parameters->workSize), NULL, NULL);
      Q_ASSERT_X (m_error == CL_SUCCESS, "cl::CommandQueue::enqueueNDRangeKernel", qPrintable (clErrorString (m_error)));
    }

  }

  void World::copyDeviceToHost( )
  {
    m_error = m_queues[0].enqueueReadBuffer (m_oDevice, CL_TRUE, 0, m_charges.size() * sizeof (double), &m_oHost[0], NULL, NULL);
    Q_ASSERT_X (m_error == CL_SUCCESS, "cl::CommandQueue::enqueueReadBuffer", qPrintable (clErrorString (m_error)));
  }

  void World::copyHostToDevice( )
  {
    m_error = m_queues[0].enqueueWriteBuffer (m_iDevice, CL_TRUE, 0, m_charges.size() * sizeof (int), &m_iHost[0], NULL, NULL);
    Q_ASSERT_X (m_error == CL_SUCCESS, "cl::CommandQueue::enqueueWriteBuffer", qPrintable (clErrorString (m_error)));

    m_error = m_queues[0].enqueueWriteBuffer (m_fDevice, CL_TRUE, 0, m_charges.size() * sizeof (int), &m_fHost[0], NULL, NULL);
    Q_ASSERT_X (m_error == CL_SUCCESS, "cl::CommandQueue::enqueueWriteBuffer", qPrintable (clErrorString (m_error)));
  }

  void World::clFinish()
  {
    m_error = m_queues[0].finish();
    Q_ASSERT_X (m_error == CL_SUCCESS, "cl::CommandQueue::finish", qPrintable (clErrorString (m_error)));
  }

}
