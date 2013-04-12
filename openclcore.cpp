#include "openclcore.h"
#include <QVariant>
#include <QDebug>
#include <QFile>

OpenCLCore::OpenCLCore(QObject *parent) :
    QObject(parent), m_error(CL_SUCCESS), m_ok(false)
{
    initialize();
}

cl::Kernel OpenCLCore::createKernel(QString fileName, QString kernelName)
{
    // obtain kernel source
    QFile file(fileName);
    if(!(file.open(QIODevice::ReadOnly|QIODevice::Text)))
    {
        throw cl::Error(-1, qPrintable(
            QString("can not open kernel source file : %1").arg(fileName)));
    }
    QByteArray contents = file.readAll();
    file.close();

    // compile program
    cl::Program::Sources source(1, std::make_pair(contents, contents.size()));
    cl::Program program(m_context, source);
    m_error = program.build(m_devices);

    // create the kernel
    return cl::Kernel(program, qPrintable(kernelName), &m_error);
}

const cl::CommandQueue& OpenCLCore::queue(int id) const
{
    return m_queues[id];
}

const cl::Device& OpenCLCore::device(int id) const
{
    return m_devices[id];
}

const cl::Platform& OpenCLCore::platform() const
{
    return m_platform;
}

const cl::Context& OpenCLCore::context() const
{
    return m_context;
}

void OpenCLCore::finish(int id) const
{
    m_queues[id].finish();
}

bool OpenCLCore::okCL()
{
    return m_ok;
}

QStringList OpenCLCore::PLATFORM_EXTENSIONS()
{
    QString property;
    try
    {
        property = QString::fromStdString(
                    m_platform.getInfo<CL_PLATFORM_EXTENSIONS>(&m_error));
    }
    catch (cl::Error error)
    {
        // something has gone wrong
        qDebug("message: CL_ERROR %d", error.err());
        qDebug("message: %s", error.what());
        qDebug("message: can not use OpenCL");

        // not ok to use OpenCL
        m_ok = false;
    }
    return property.split(" ", QString::SkipEmptyParts);
}

QString OpenCLCore::PLATFORM_PROFILE()
{
    QString property;
    try
    {
        property = QString::fromStdString(
                    m_platform.getInfo<CL_PLATFORM_PROFILE>(&m_error));
    }
    catch (cl::Error error)
    {
        // something has gone wrong
        qDebug("message: CL_ERROR %d", error.err());
        qDebug("message: %s", error.what());
        qDebug("message: can not use OpenCL");

        // not ok to use OpenCL
        m_ok = false;
    }
    return property;
}

QString OpenCLCore::PLATFORM_VERSION()
{
    QString property;
    try
    {
        property = QString::fromStdString(
                    m_platform.getInfo<CL_PLATFORM_VERSION>(&m_error));
    }
    catch (cl::Error error)
    {
        // something has gone wrong
        qDebug("message: CL_ERROR %d", error.err());
        qDebug("message: %s", error.what());
        qDebug("message: can not use OpenCL");

        // not ok to use OpenCL
        m_ok = false;
    }
    return property;
}

QString OpenCLCore::PLATFORM_VENDOR()
{
    QString property;
    try
    {
        property = QString::fromStdString(
                    m_platform.getInfo<CL_PLATFORM_VENDOR>(&m_error));
    }
    catch (cl::Error error)
    {
        // something has gone wrong
        qDebug("message: CL_ERROR %d", error.err());
        qDebug("message: %s", error.what());
        qDebug("message: can not use OpenCL");

        // not ok to use OpenCL
        m_ok = false;
    }
    return property;
}

QString OpenCLCore::PLATFORM_NAME()
{
    QString property;
    try
    {
        property = QString::fromStdString(
                    m_platform.getInfo<CL_PLATFORM_NAME>(&m_error));
    }
    catch (cl::Error error)
    {
        // something has gone wrong
        qDebug("message: CL_ERROR %d", error.err());
        qDebug("message: %s", error.what());
        qDebug("message: can not use OpenCL");

        // not ok to use OpenCL
        m_ok = false;
    }
    return property;
}

void OpenCLCore::initialize()
{
    // not ok to use OpenCL
    m_ok = false;

    try
    {
        // get list of platforms
        std::vector<cl::Platform> platforms;
        m_error = cl::Platform::get(&platforms);

        // select which platform to use
        if (platforms.size() != 1)
        {
            if (platforms.size() == 0)
            {
                throw cl::Error(m_error, "empty platform vector");
            }
            qDebug("message: multiple platforms found");
        }
        m_platform = platforms[0];

        // set up context properties
        cl_context_properties context_properties[3] = {
            CL_CONTEXT_PLATFORM, (cl_context_properties)(m_platform)(), 0
        };

        // create context
        m_context = cl::Context(CL_DEVICE_TYPE_GPU, context_properties, NULL,
                                NULL, &m_error);

        // get device list
        m_devices = m_context.getInfo<CL_CONTEXT_DEVICES>(&m_error);

        // create queue for each device
        for (unsigned int i = 0; i < m_devices.size(); i++)
        {
            m_queues.push_back(cl::CommandQueue(m_context, m_devices[i], 0,
                                                &m_error));
            m_error = m_queues[i].finish();
        }
    }
    catch (cl::Error error)
    {
        // something has gone wrong
        qDebug("message: CL_ERROR %d", error.err());
        qDebug("message: %s", error.what());
        qDebug("message: can not use OpenCL");

        // not ok to use OpenCL
        m_ok = false;
    }

    // ok to use OpenCL
    m_ok = true;
}

QDebug operator<<(QDebug dbg, OpenCLCore& core)
{
    dbg.nospace()
        << qPrintable(
               QString("%1 : %2")
                  .arg("CL_PLATFORM_NAME", -30)
                  .arg(core.PLATFORM_VENDOR())
           )
        << '\n'
        << qPrintable(
               QString("%1 : %2")
                  .arg("CL_PLATFORM_NAME", -30)
                  .arg(core.PLATFORM_VENDOR())
           )
        << '\n'
        << qPrintable(
               QString("%1 : %2")
                  .arg("CL_PLATFORM_VERSION", -30)
                  .arg(core.PLATFORM_VERSION())
           )
        << '\n'
    ;
    return dbg.nospace();
}
