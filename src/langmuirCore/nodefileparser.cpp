#include "nodefileparser.h"
#include <QThreadPool>
#include <QRegExp>
#include <QFile>

namespace Langmuir {

NodeFileParser::NodeFileParser(const QString &path, QObject *parent) :
    QObject(parent)
{
    setPath(path);
}

void NodeFileParser::setPath(const QString &path)
{
    if (path.isEmpty())
    {
        char * PBS_NODEFILE = getenv("PBS_NODEFILE");

        if (PBS_NODEFILE == NULL)
        {
            m_path = "";
        }
        else
        {
            m_path = QString(PBS_NODEFILE);
        }
    }
    else
    {
        m_path = path;
    }
    parse();
}

void NodeFileParser::setDefault()
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

void NodeFileParser::parse()
{
    if (m_path.isEmpty())
    {
        setDefault();
        return;
    }
    else
    {
        m_names.clear();
        m_cores.clear();

        QFile file;
        file.setFileName(m_path);
        if (!file.open(QIODevice::ReadOnly))
        {
            qDebug("nodefile: can not open file: %s", qPrintable(m_path));
            setDefault();
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

    for (int i = 0; i < m_names.size(); i++) {
        qDebug("nodefile: %s cores=%d", qPrintable(m_names.at(i)), m_cores.at(i));
    }
}

const QStringList& NodeFileParser::names()
{
    return m_names;
}

const QVector<int>& NodeFileParser::cores()
{
    return m_cores;
}

QString NodeFileParser::namesAt(int i)
{
    return m_names.at(i);
}

int NodeFileParser::coresAt(QString name)
{
    return m_cores.at(m_names.indexOf(name));
}

int NodeFileParser::coresAt(int i)
{
    return m_cores.at(i);
}

int NodeFileParser::numHosts()
{
    return m_names.size();
}

int NodeFileParser::numProcesses()
{
    int processes = 0;

    for (int i = 0; i < m_cores.size(); i++) {
        processes += m_cores.at(i);
    }

    return processes;
}

}
