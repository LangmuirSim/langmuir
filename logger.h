#ifndef LOGGER_H
#define LOGGER_H

#include <QTextStream>
#include <QPainter>
#include <QObject>
#include <QImage>
#include <QDir>

namespace Langmuir
{

class World;
class ChargeAgent;
class FluxAgent;

class DataStream : public QTextStream
{
public:
    DataStream(QString name, int width = 13, int precision = 5);
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
    GridImage(World &world, QColor bg = Qt::white, QObject *parent=0);
    void drawSites(QList<int> &sites, QColor color, int layer);
    void drawCharges(QList<ChargeAgent *> &charges,QColor color, int layer);
    void save(QString name, int scale=5);
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
    ~Logger();

    void saveElectronIDs();
    void saveHoleIDs();
    void saveTrapIDs();
    void saveDefectIDs();
    void saveElectronGridPotential();
    void saveHoleGridPotential();
    void saveCoulombEnergy();

    void saveTrapImage();
    void saveDefectImage();
    void saveElectronImage();
    void saveHoleImage();
    void saveImage();

    void report(ChargeAgent &charge);
    void reportFluxStream();

    QString generateFileName(QString identifier, QString extension = "dat", bool sim = true, bool step = true);

private:
    World &m_world;
    QDir m_outputdir;
    DataStream *m_fluxStream;
    DataStream *m_carrierStream;
};

}
#endif // LOGGER_H
