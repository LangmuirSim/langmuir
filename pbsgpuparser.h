#ifndef PBSGPUPARSER_H
#define PBSGPUPARSER_H

#include <QObject>
#include <QList>

namespace Langmuir {

/**
 * A class to parse the PBS_GPUFILE - needed for selecting the correct GPU on a cluster using PBS
 */
class PBSGPUParser : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief create the PBSGPUParser
     */
    explicit PBSGPUParser(QObject *parent = 0);

    /**
     * @brief set the path of the PBS_GPUFILE
     */
    void setPath(const QString& path = "");

    /**
     * @brief get the path of the PBS_GPUFILE from the ENVIRONMENT variables
     */
    void aquirePath();

    /**
     * @brief reset to default GPU(0)
     */
    void setDefault();

    /**
     * @brief parse the PBS_GPUFILE
     */
    void parse();

    /**
     * @brief get the list of nodes ids found
     */
    QList<int> nodes();

    /**
     * @brief get the list of GPU ids found
     */
    QList<int> gpus();

    /**
     * @brief get the number of GPUs found
     */
    int size();

    /**
     * @brief get the first node found
     */
    int node();

    /**
     * @brief get the first GPU found
     */
    int gpu();

private:
    //! list of nodes
    QList<int> m_nodes;

    //! list of GPU ids
    QList<int> m_gpus;

    //! path to PBS_GPUFILE
    QString m_path;
};

}
#endif // PBSGPUPARSER_H
