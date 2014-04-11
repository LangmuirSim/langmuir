#ifndef NODEFILEPARSER_H
#define NODEFILEPARSER_H

#include <QStringList>
#include <QVector>
#include <QString>
#include <QObject>
#include <QDebug>

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
     * @param path path to NODEFILE
     */
    explicit NodeFileParser(const QString& path="", QObject *parent = 0);

    /**
     * @brief set the path of the NODEFILE
     * @param path path to NODEFILE
     */
    void setPath(const QString& path = "");

    /**
     * @brief set the default based on QThreadPool
     */
    void setDefault();

    /**
     * @brief get the list of cpu hostnames
     */
    const QStringList& names();

    /**
     * @brief get the list of cpu core counts
     */
    const QVector<int>& cores();

    /**
     * @brief get the cpu hostname by index
     * @param i index
     */
    QString namesAt(int i);

    /**
     * @brief get the number of cores on a cpu by hostname
     */
    int coresAt(QString name);

    /**
     * @brief get the number of cores on a cpu by index
     * @param i index
     */
    int coresAt(int i);

    /**
     * @brief get the total number of CPUs
     */
    int numHosts();

    /**
     * @brief get the total number of threads
     */
    int numProcesses();

private:
    //! list of cpu names
    QStringList  m_names;

    //! list of core counts per cpu
    QVector<int> m_cores;

    //! path to NODEFILE
    QString m_path;

    /**
     * @brief parse the NODEFILE
     */
    void parse();
};

}

#endif // NODEFILEPARSER_H
