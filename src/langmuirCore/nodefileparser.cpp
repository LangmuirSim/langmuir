#include "nodefileparser.h"
#include <QThreadPool>
#include <QRegExp>
#include <QFile>

namespace Langmuir {

NodeFileParser::NodeFileParser(const QString &nodefile, const QString &gpufile, QObject *parent) :
    QObject(parent)
{
    setNodeFile(nodefile);
    setGPUFile(gpufile);
}

void NodeFileParser::setNodeFile(const QString &path)
{
    if (path.isEmpty())
    {
        char * PBS_NODEFILE = getenv("PBS_NODEFILE");

        if (PBS_NODEFILE == NULL)
        {
            m_nodefile = "";
        }
        else
        {
            m_nodefile = QString(PBS_NODEFILE);
        }
    }
    else
    {
        m_nodefile = path;
    }
}

void NodeFileParser::setGPUFile(const QString &path)
{
    if (path.isEmpty())
    {
        char * PBS_GPUFILE = getenv("PBS_GPUFILE");

        if (PBS_GPUFILE == NULL)
        {
            m_gpufile = "";
        }
        else
        {
            m_gpufile = QString(PBS_GPUFILE);
        }
    }
    else
    {
        m_gpufile = path;
    }
    parseGPUFile();
}

void NodeFileParser::setNodeDefault()
{
    m_names.clear();
    m_cores.clear();

    QThreadPool& threadPool = *QThreadPool::globalInstance();
    int maxThreadCount = threadPool.maxThreadCount();

    m_names.append("node1");
    m_cores.append(maxThreadCount);

    qDebug("nodefile: setting defaults to hostname=%s, cores=%d",
        qPrintable(m_names.at(0)), m_cores.at(0));
}

void NodeFileParser::setGPUDefault()
{
}

void NodeFileParser::parse()
{
}

void NodeFileParser::parseNodeFile()
{
    QFile file;
    file.setFileName(m_nodefile);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug("nodefile: can not open file: %s", qPrintable(m_nodefile));
        return;
    }

    QRegExp regex("(\\w+)(:(\\d+))?(\\s*slots\\s*=\\s*(\\d+)\\s*)?.*$");
    bool ok = true;

    while (!file.atEnd())
    {
        QString line = file.readLine().trimmed();

        if (regex.exactMatch(line))
        {
            QString name = regex.cap(1);

            if (!m_names.contains(name)) {
                m_names.append(name);
                m_cores.append(0);
            }

            if (!regex.cap(3).isEmpty()) {
                m_cores[m_names.indexOf(name)] += regex.cap(3).toInt(&ok);
                if (!ok) {
                    qFatal("nodefile: can not parse line: %s", qPrintable(line));
                }
                continue;
            }

            if (!regex.cap(5).isEmpty()) {
                m_cores[m_names.indexOf(name)] += regex.cap(5).toInt(&ok);
                if (!ok) {
                    qFatal("nodefile: can not parse line: %s", qPrintable(line));
                }
                continue;
            }

            m_cores[m_names.indexOf(name)] += 1;
        }
        else
        {
            qFatal("nodefile: failed to match line! %s", qPrintable(line));
        }
    }
}

void NodeFileParser::parseGPUFile()
{
    QFile file;
    file.setFileName(m_gpufile);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug("nodefile: can not open file: %s", qPrintable(m_nodefile));
        return;
    }

    QRegExp regex1("^(\\w+)(:(\\d+))?-?.*$");
    QRegExp regex2("^.*gpus?\\s*=?\\s*(\\d+).*$");
    QRegExp regex3("^.*slots?\\s*=?\\s*(\\d+).*$");
    bool ok = true;

    while (!file.atEnd())
    {
        QString line = file.readLine().trimmed();

        regex1.exactMatch(line);
        regex2.exactMatch(line);
        regex3.exactMatch(line);
    }
}

void NodeFileParser::check()
{
}

}
