#ifndef OPENCLCORE_H
#define OPENCLCORE_H

#include <QStringList>
#include <QString>
#include <QObject>
#include <vector>

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

//! A class to manage OpenCL
class OpenCLCore : public QObject
{
    Q_OBJECT
public:
    //! initialize OpenCL
    explicit OpenCLCore(QObject *parent = 0);

    //! create kernel from file
    cl::Kernel createKernel(QString fileName, QString kernelName);

    //! get command queue for a device
    const cl::CommandQueue& queue(int id = 0) const;

    //! get a device
    const cl::Device& device(int id = 0) const;

    //! get the platform
    const cl::Platform& platform() const;

    //! get the context
    const cl::Context& context() const;

    //! tell a queue to finish all tasks
    void finish(int id = 0) const;

    //! initialize OpenCL
    void initialize();

    //! find out if OpenCL can be used
    bool okCL();

    QStringList PLATFORM_EXTENSIONS();
    QString PLATFORM_PROFILE();
    QString PLATFORM_VERSION();
    QString PLATFORM_VENDOR();
    QString PLATFORM_NAME();

    //! show OpenCL info
    friend QDebug operator<<(QDebug dbg, OpenCLCore& core);

private:
    //! list of queues
    std::vector<cl::CommandQueue> m_queues;

    //! list of devices
    std::vector<cl::Device> m_devices;

    //! the platform
    cl::Platform m_platform;

    //! the context
    cl::Context m_context;

    //! error code
    cl_int m_error;

    //! OpenCL is ok to use
    bool m_ok;
};

#endif // OPENCLHELPER_H
