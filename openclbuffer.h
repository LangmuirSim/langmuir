#ifndef OPENCLBUFFER_H
#define OPENCLBUFFER_H

#include <QObject>
#include <QVector>
#include <QDebug>

#include "openclcore.h"

//! A base class for various OpenCL memory types
class OpenCLMemory : public QObject
{
    Q_OBJECT
public:
    //! device memory access rights
    enum MemoryMode
    {
        ReadOnly  = 0,
        WriteOnly = 1,
        ReadWrite = 2
    };
    Q_ENUMS(MemoryMode)

    //! reference the core and make it the parent QObject
    explicit OpenCLMemory(OpenCLCore &core);

protected:
    //! the OpenCL core objects
    OpenCLCore& m_core;
};

template <class T>
//! A class to manage a basic OpenCL buffer
class OpenCLBuffer : public OpenCLMemory
{
public:
    //! set up buffer
    explicit OpenCLBuffer(unsigned int size, MemoryMode mode, OpenCLCore &core);

    //! copy from device to host
    void readFromDevice(int id=0, bool block=true);

    //! copy from host to device
    void writeToDevice(int id=0, bool block=true);

    //! access host vector
    const T& operator[](unsigned int i) const;

    //! access host vector
    T& operator[](unsigned int i);

    //! get device buffer
    cl::Buffer& deviceMemory();

    //! get host vector
    QVector<T>& hostMemory();

    //! get the number of bits
    unsigned int bits();

    //! get the host vector size
    unsigned int size();

protected:
    //! the device buffer
    cl::Buffer m_memoryDevice;

    //! the host vector
    QVector<T> m_memoryHost;

    //! the number of bits
    unsigned int m_bits;
};

inline OpenCLMemory::OpenCLMemory(OpenCLCore &core)
     : QObject(&core), m_core(core)
{
}

template <class T> OpenCLBuffer<T>::OpenCLBuffer(unsigned int size,
                                                 MemoryMode mode,
                                                 OpenCLCore &core)
    : OpenCLMemory(core)
{
    setObjectName("OpenCLBuffer"); 
    m_memoryHost.clear();
    m_memoryHost.resize(size);
    m_bits = size * sizeof(T);

    switch (mode)
    {
        case ReadOnly:
        {
            m_memoryDevice = cl::Buffer(m_core.context(), CL_MEM_READ_ONLY,
                                        m_bits);
            break;
        }

        case WriteOnly:
        {
            m_memoryDevice = cl::Buffer(m_core.context(), CL_MEM_WRITE_ONLY,
                                        m_bits);
            break;
        }

        case ReadWrite:
        {
            m_memoryDevice = cl::Buffer(m_core.context(), CL_MEM_READ_WRITE,
                                        m_bits);
            break;
        }

        default:
        {
            throw cl::Error(-1, "invalid memory flag");
            break;
        }
    }
}

template <class T> void OpenCLBuffer<T>::writeToDevice(int id, bool block)
{
    m_core.queue(id).enqueueWriteBuffer(m_memoryDevice, block, 0, m_bits,
                                        &m_memoryHost[0]);
}

template <class T> void OpenCLBuffer<T>::readFromDevice(int id, bool block)
{
    m_core.queue(id).enqueueReadBuffer(m_memoryDevice, block, 0, m_bits,
                                       &m_memoryHost[0]);
}

template <class T> const T& OpenCLBuffer<T>::operator[](unsigned int i) const
{
    return m_memoryHost[i];
}

template <class T> T& OpenCLBuffer<T>::operator[](unsigned int i)
{
    return m_memoryHost[i];
}

template <class T> cl::Buffer& OpenCLBuffer<T>::deviceMemory()
{
    return m_memoryDevice;
}

template <class T> QVector<T>& OpenCLBuffer<T>::hostMemory()
{
    return m_memoryHost;
}

template <class T> unsigned int OpenCLBuffer<T>::bits()
{
    return m_bits;
}

template <class T> unsigned int OpenCLBuffer<T>::size()
{
    return m_memoryHost.size();
}

#endif // OPENCLBUFFER_H
