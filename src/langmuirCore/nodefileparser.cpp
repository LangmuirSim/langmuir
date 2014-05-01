#include "nodefileparser.h"
#include <QThreadPool>
#include <QRegExp>
#include <QFile>

#include <boost/asio.hpp>

namespace Langmuir {

NodeFileParser::NodeFileParser(const QString &nodefile, const QString &gpufile, QObject *parent) :
    QObject(parent)
{
    setPaths(nodefile, gpufile);
    setHostName();
}

void NodeFileParser::setHostName() {
    m_hostName = QString::fromStdString(boost::asio::ip::host_name());
    qDebug("langmuir: hostname=%s", qPrintable(m_hostName));
}

void NodeFileParser::setHostName(const QString &hostName) {
    m_hostName = hostName;
    qDebug("langmuir: hostname=%s", qPrintable(m_hostName));
}

void NodeFileParser::setPaths(const QString &nodefile, const QString &gpufile)
{
    // set the NODEFILE path
    if (nodefile.isEmpty())
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
        m_nodefile = nodefile;
    }

    // set the GPUFILE path
    if (gpufile.isEmpty())
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
        m_gpufile = gpufile;
    }

    // clear the records
    clear();

    if (!m_nodefile.isEmpty()) {
        qDebug("langmuir: PBS_NODEFILE %s", qPrintable(m_nodefile));
    }
    else {
        qDebug("langmuir: can not aquire PBS_NODEFILE");
    }

    if (!m_gpufile.isEmpty()) {
        qDebug("langmuir: PBS_GPUFILE  %s", qPrintable(m_gpufile ));
    }
    else {
        qDebug("langmuir: can not aquire PBS_GPUFILE");
    }

    // we have both files
    if (!m_nodefile.isEmpty() && !m_gpufile.isEmpty())
    {
        qDebug("langmuir: parsing nodefile");
        bool parseN = parse(m_nodefile, false, true);

        qDebug("langmuir: parsing gpufile");
        bool parseG = parse(m_gpufile , parseN, false);

        if (!parseN && !parseG) {
            qDebug("langmuir: failed to parse NODEFILE and GPUFILE...");
            setDefault();
        }
        return;
    }

    // we are missing the GPUFILE
    if (m_gpufile.isEmpty() && !m_nodefile.isEmpty())
    {
        qDebug("langmuir: parsing NODEFILE");
        bool parseN = parse(m_nodefile, false, false);

        if (!parseN) {
            qDebug("langmuir: failed to parse NODEFILE");
            setDefault();
        }
        return;
    }

    // we are missing the NODEFILE
    if (m_nodefile.isEmpty() && !m_gpufile.isEmpty())
    {
        qDebug("langmuir: parsing GPUFILE");
        bool parseG = parse(m_gpufile , false, false);

        if (!parseG) {
            qDebug("langmuir: failed to parse GPUFILE");
            setDefault();
        }
        return;
    }

    // we are missing both files
    setDefault();
}

void NodeFileParser::setDefault()
{
    qDebug("langmuir: setting core/gpu defaults");

    QString name = "node1";

    QThreadPool& threadPool = *QThreadPool::globalInstance();
    int maxThreadCount = threadPool.maxThreadCount();

    clear();
    createNode("node1");
    m_cores[name] = maxThreadCount;
}

void NodeFileParser::createNode(const QString &name, int cores, QList<int> gpus)
{
    if (!m_names.contains(name))
    {
        m_names.append(name);
    }

    if (!m_cores.contains(name))
    {
        m_cores[name] = cores;
    }

    if (!m_gpus.contains(name))
    {
        m_gpus[name] = gpus;
    }
}

bool NodeFileParser::parse(QString& filename, bool ignoreCores, bool ignoreGPUs)
{
    QFile file;
    file.setFileName(filename);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug("langmuir: can not open file: %s", qPrintable(filename));
        return false;
    }

    // expression to match name and core count
    QRegExp regex1("^\\s*(\\w+)(:(\\d+))?.*\\s*(slots\\s*=\\s*(\\d+))?.*$");

    // expression to match gpu count
    QRegExp regex2("^.*gpus?\\s*=?\\s*(\\d+).*$");

    bool ok = true;

    while (!file.atEnd())
    {
        QString line = file.readLine().trimmed();

        if (regex1.exactMatch(line))
        {
            // find the name of the node
            QString name = regex1.cap(1);
            createNode(name);

            // the default number of cores when no pattern matches is 1
            int cores = 1;

            // look for core patterns
            if (!regex1.cap(3).isEmpty())
            {
                // using the form name:cores
                cores = regex1.cap(3).toInt(&ok);
                if (!ok)
                {
                    qFatal("langmuir: can not parse name:(cores) in line: %s",
                           qPrintable(line));
                }
            }
            else if (!regex1.cap(5).isEmpty())
            {
                // using the form slots=cores
                cores = regex1.cap(5).toInt(&ok);
                if (!ok)
                {
                    qFatal("langmuir: can not parse slots=(cores) in line: %s",
                           qPrintable(line));
                }
            }
            else if (ignoreCores)
            {
                // we ignore cores in the GPU file
                cores = 0;
            }

            // update the number of cores
            m_cores[name] += cores;

            // look for gpu patterns
            if (!ignoreGPUs)
            {
                if (regex2.exactMatch(line))
                {
                    int gpu_id = regex2.cap(1).toInt(&ok);
                    if (!ok)
                    {
                        qFatal("langmuir: can not parse -?gpus?=(id) in line: %s",
                               qPrintable(line));
                    }
                    if (!m_gpus[name].contains(gpu_id))
                    {
                        m_gpus[name].append(gpu_id);
                    }
                }
            }
        }
        else
        {
            qFatal("langmuir: can not match line: %s", qPrintable(line));
        }
    }
    return true;
}

void NodeFileParser::clear()
{
    m_names.clear();
    m_cores.clear();
    m_gpus.clear();
}

QDebug operator<<(QDebug dbg, const NodeFileParser& nfp)
{
    QString line = "%1 cores=%2 gpus=%3\n";

    foreach(QString name, nfp.m_names)
    {
        dbg.nospace() << qPrintable(line.arg(name).arg(nfp.m_cores[name])
            .arg(nfp.m_gpus[name].size()));
    }

    return dbg.nospace();
}

int NodeFileParser::numProc()
{
    int count = 0;
    foreach (QString name, m_names) {
        count += m_cores[name];
    }
    return count;
}

int NodeFileParser::numProc(const QString& name)
{
    return m_cores[name];
}

const QMap<QString, int> &NodeFileParser::procs()
{
    return m_cores;
}

int NodeFileParser::numGPUS()
{
    int count = 0;
    foreach (QString name, m_names) {
        count += m_gpus[name].size();
    }
    return count;
}

int NodeFileParser::numGPUs(const QString& name)
{
    return m_gpus[name].size();
}

int NodeFileParser::GPUid(const QString &name, int i)
{
    return m_gpus[name].at(i);
}

const QList<int>& NodeFileParser::gpus(const QString& name)
{
    return m_gpus[name];
}

int NodeFileParser::numCPUs()
{
    return m_names.size();
}

const QStringList& NodeFileParser::cpus()
{
    return m_names;
}

const QString& NodeFileParser::hostName()
{
    if (m_names.contains(m_hostName)) {
        return m_hostName;
    }

    if (m_names.size() > 0) {
        m_hostName = m_names.at(0);
        return m_hostName;
    }

    qFatal("langmuir: invalid hostname!");
}

}
