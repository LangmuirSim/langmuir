#ifndef LOGGER_H
#define LOGGER_H

#include <QtCore>

namespace Langmuir
{
class World;
class Agent;

/**
  *  @class Logger
  *  @brief For writing simulation information to Files
  */
class Logger : public QObject
{
Q_OBJECT public:
    Logger(World *world, QObject *parent=0);
    ~Logger();

    /**
      * @brief Save \b current carrier site ids
      * @param name name of file
      */
    void saveCarrierIDsToFile(QString name);

    /**
      * @brief Save trap site ids
      * @param name name of file
      */
    void saveTrapIDsToFile(QString name);

    /**
      * @brief Save defect site ids
      * @param name name of file
      */
    void saveDefectIDsToFile(QString name);

    /**
      * @brief Save potential value at every site
      * @param name name of file
      */
    void saveFieldEnergyToFile(QString name);

    /**
      * @brief Save \b current coulomb energy
      * @param name name of file
      * @note it is saving whatever is in m_oHost vector
      */
    void saveCoulombEnergyToFile(QString name);

    /**
      * @brief Save image of trape sites
      * @param name name of file
      * @note uses trap IDs
      */
    void saveTrapImageToFile(QString name);

    /**
      * @brief Save carrier pathlength and lifetime to carrier output stream
      * @param i index of carrier
      * @param tick number identifing when output is written, for example the simulation step
      * @warning doesn't not check if \b i is in range
      */
    void carrierReportLifetimeAndPathlength(int i, int tick);

    /**
      * @brief flush the carrier stream file
      * @note called in destructor of logger to make sure this is done at least once
      */
    void carrierStreamFlush();

private:

    /**
     * @brief address of the world object
     */
    World *m_world;

    /**
     * @brief directory to locate output files in
     */
    QDir m_outputdir;

    /**
     * @brief output file for pathlength and lifetime of carriers
     */
    QFile m_carrierFile;

    /**
     * @brief output stream for pathlength and lifetime of carriers
     */
    QTextStream m_carrierStream;
};
}
#endif // LOGGER_H
