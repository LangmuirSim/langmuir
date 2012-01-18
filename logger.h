#ifndef LOGGER_H
#define LOGGER_H

#include <QtCore>

namespace Langmuir
{
class World;
class Agent;
class ChargeAgent;

/**
  *  @class Logger
  *  @brief For writing simulation information to Files
  */
class Logger : public QObject
{
Q_OBJECT public:
    Logger(World *world, QObject *parent=0);
    ~Logger();

    void saveElectronIDs();
    void saveHoleIDs();
    void saveTrapIDs();
    void saveDefectIDs();
    void saveElectronGridPotential();
    void saveHoleGridPotential();
    void saveCoulombEnergy();
    void saveTrapImage();

    void holeReportPathlength(int id);
    void holeReportLifetime(int id);
    void electronReportPathlength(int id);
    void electronReportLifetime(int id);

    void holePathlengthFlush();
    void holeLifetimeFlush();
    void electronPathlengthFlush();
    void electronLifetimeFlush();

    void flush();
    void report( ChargeAgent &charge );

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
     * @brief save files with this stub, simulation specific
     */
    QString m_stub;

    QFile m_carrierFile;
    QTextStream m_carrierStream;

    QFile m_holeLifetimeFile;
    QFile m_holePathlengthFile;
    QFile m_electronLifetimeFile;
    QFile m_electronPathlengthFile;

    QTextStream m_holeLifetimeStream;
    QTextStream m_holePathlengthStream;
    QTextStream m_electronLifetimeStream;
    QTextStream m_electronPathlengthStream;
};
}
#endif // LOGGER_H
