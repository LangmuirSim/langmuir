#ifndef LOGGER_H
#define LOGGER_H

#include <QTextStream>
#include <QPainter>
#include <QObject>
#include <QImage>
#include <QDir>
#include <QDebug>

namespace Langmuir
{

class World;
class ChargeAgent;
class FluxAgent;

class DataStream : public QTextStream
{
public:
    DataStream(QString name, int width = 13, int precision = 5,
               QIODevice::OpenMode openMode = QIODevice::WriteOnly|QIODevice::Text,
               bool mustNotExist = true, bool deleteBeforeOpening = false);
    void newline();
    void setStreamParameters();
    void fillColumns(int count, QString fill = "-");
private:
    QFile m_file;
    int m_width;
    int m_precision;
};

class GridImage : public QObject
{
    Q_OBJECT
public:
    GridImage(World &world, QColor bg = Qt::black, QObject *parent=0);
    void drawSites(QList<int> &sites, QColor color, int layer);
    void drawCharges(QList<ChargeAgent *> &charges,QColor color, int layer);
    void save(QString name, int scale=3);
private:
    QPainter m_painter;
    QImage m_image;
    World &m_world;
};

class Logger : public QObject
{
    Q_OBJECT
public:
    Logger(World &world, QObject *parent = 0);
    virtual ~Logger();

    virtual void initialize();
    virtual void saveElectronIDs();
    virtual void saveHoleIDs();
    virtual void saveTrapIDs();
    virtual void saveDefectIDs();
    virtual void saveElectronGridPotential();
    virtual void saveHoleGridPotential();
    virtual void saveCoulombEnergy();
    virtual void saveTrapImage();
    virtual void saveDefectImage();
    virtual void saveElectronImage();
    virtual void saveHoleImage();
    virtual void saveImage();
    virtual void report(ChargeAgent &charge);
    virtual void reportFluxStream();
    virtual void reportXYZStream();
    virtual QString generateFileName(QString identifier, QString extension = "dat", bool sim = true, bool step = true);

protected:
    World &m_world;
    QDir m_outputdir;
    DataStream *m_fluxStream;
    DataStream *m_carrierStream;
    DataStream *m_xyzStream;
};

class NullLogger : public Logger
{
public:
    NullLogger(World &world, QObject *parent = 0) : Logger(world,parent) { }
    ~NullLogger() {}

    virtual void initialize() {}
    virtual void saveElectronIDs() {}
    virtual void saveHoleIDs() {}
    virtual void saveTrapIDs() {}
    virtual void saveDefectIDs() {}
    virtual void saveElectronGridPotential() {}
    virtual void saveHoleGridPotential() {}
    virtual void saveCoulombEnergy() {}
    virtual void saveTrapImage() {}
    virtual void saveDefectImage() {}
    virtual void saveElectronImage() {}
    virtual void saveHoleImage() {}
    virtual void saveImage() {}
    virtual void report(ChargeAgent &charge) {}
    virtual void reportFluxStream() {}
    virtual void reportXYZStream() {}
    virtual QString generateFileName(QString identifier, QString extension = "dat", bool sim = true, bool step = true)
    {
        qFatal("NullLogger::generateFileName");
        return QString();
    }
};

}
#endif // LOGGER_H
