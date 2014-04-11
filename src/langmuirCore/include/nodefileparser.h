#ifndef NODEFILEPARSER_H
#define NODEFILEPARSER_H

#include <QStringList>
#include <QVector>
#include <QString>
#include <QObject>
#include <QDebug>
#include <QMap>

namespace Langmuir {

/**
 * A class to parse the PBS_NODEFILE
 */
class NodeFileParser : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief create NodeFileParser
     * @param nodefile path to NODEFILE
     * @param gpufile path to GPUFILE
     */
    explicit NodeFileParser(const QString &nodefile = "", const QString &gpufile = "", QObject *parent = 0);

    /**
     * @brief set the path of the NODEFILE
     * @param path path to NODEFILE
     */
    void setNodeFile(const QString& path = "");

    /**
     * @brief set the path of the GPUFILE
     * @param path path to GPUFILE
     */
    void setGPUFile(const QString& path = "");

    /**
     * @brief parse the NODEFILE and GPUFILE
     */
    void parse();

private:
    //! list of cpu names
    QStringList  m_names;

    //! list of core counts per cpu
    QVector<int> m_cores;

    //! list of gpu ids per cpu
    QMap<QString,QVector<int> > m_gpus;

    //! path to NODEFILE
    QString m_nodefile;

    //! path to GPUFILE
    QString m_gpufile;

    /**
     * @brief set the default based on QThreadPool
     */
    void setNodeDefault();

    /**
     * @brief set the default GPU
     */
    void setGPUDefault();

    /**
     * @brief parseNodeFile
     */
    void parseNodeFile();

    /**
     * @brief parseGPUFile
     */
    void parseGPUFile();

    /**
     * @brief check
     */
    void check();
};

}

#endif // NODEFILEPARSER_H
