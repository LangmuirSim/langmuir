#include "pbsgpuparser.h"
#include <QRegExp>
#include <QDebug>
#include <QFile>

namespace Langmuir {

PBSGPUParser::PBSGPUParser(QObject *parent) :
    QObject(parent)
{
    aquirePath();
    if (!m_path.isEmpty())
    {
        parse();
    }
    else
    {
        m_nodes.push_back(0);
        m_gpus.push_back(0);
    }
}


void PBSGPUParser::setPath(const QString& path)
{
    m_path = path;
}

void PBSGPUParser::aquirePath()
{
    char * PBS_GPUFILE = getenv("PBS_GPUFILE");
    if (PBS_GPUFILE == NULL)
    {
        m_path = "";
    }
    else
    {
        m_path = QString(PBS_GPUFILE);
    }
}

void PBSGPUParser::setDefault()
{
    m_path = "";
    m_gpus.clear();
    m_gpus.push_back(0);
    m_nodes.clear();
    m_nodes.push_back(0);
    qDebug("langmuir: assuming default GPU=0");
}

void PBSGPUParser::parse()
{
    m_nodes.clear();
    m_gpus.clear();

    if (m_path.isEmpty())
    {
        qDebug("langmuir: path is NULL");
        setDefault();
        return;
    }

    QFile file;
    file.setFileName(m_path);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug("langmuir: could not open %s", qPrintable(m_path));
        setDefault();
        return;
    }

    while (!file.atEnd())
    {
        QString line = file.readLine().trimmed();
        QRegExp regex("\\D+(\\d+)-gpu(\\d+)");
        bool ok = true;

        if (regex.exactMatch(line))
        {
            int node_id = regex.cap(1).toInt(&ok);
            if (!ok)
            {
                qDebug("langmuir: failed to parse \"%s\"",
                         qPrintable(line));
                setDefault();
                return;
            }
            m_nodes.push_back(node_id);

            int gpu_id = regex.cap(2).toInt(&ok);
            if (!ok)
            {
                qDebug("langmuir: failed to parse \"%s\"",
                         qPrintable(line));
                setDefault();
                return;
            }
            m_gpus.push_back(gpu_id);

            qDebug("langmuir: found NODE=%d, GPU=%d", node_id, gpu_id);
        }
        else
        {
            qDebug("langmuir: failed to match \"%s\"",
                   qPrintable(line));
            setDefault();
            return;
        }
    }

    qDebug("langmuir: found %d GPUs", m_gpus.size());
}

QList<int> PBSGPUParser::nodes()
{
    return m_nodes;
}

QList<int> PBSGPUParser::gpus()
{
    return m_gpus;
}

int PBSGPUParser::size()
{
    return m_nodes.size();
}

int PBSGPUParser::node()
{
    return m_nodes.at(0);
}

int PBSGPUParser::gpu()
{
    return m_gpus.at(0);
}

}
