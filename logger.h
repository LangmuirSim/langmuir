#ifndef LOGGER_H
#define LOGGER_H

#include <QTextStream>
#include <QPainter>
#include <QObject>
#include <QDir>

namespace Langmuir
{

class DataColumnBase : public QObject
{
    Q_OBJECT
public:
    DataColumnBase(QString title = 0,QObject *parent = 0)
        : QObject(parent), m_title(title) {}
    inline QString& title() { return m_title; }
    virtual void report( QTextStream& stream ) = 0;
private:
    QString m_title;
};

template <class T>
class DataColumn : public DataColumnBase
{
public:
    DataColumn(T& parameter, QString title = "", QObject *parent = 0)
        : DataColumnBase(title,parent), m_parameter(&parameter) {}
    inline virtual void report( QTextStream &stream ) { stream << *m_parameter; }
private:
    T *m_parameter;
};

class DataFile : public QObject
{
    Q_OBJECT
public:
    DataFile( QString name, int width = 13, int percision = 5, QObject *parent = 0);
    template <typename T> void addColumn( T& parameter, QString title )
    {
        DataColumnBase *col = new DataColumn<T>(parameter,title);
        m_columns.push_back(col);
    }
    void report();
private:
    QFile m_file;
    QTextStream m_stream;
    QList<DataColumnBase*> m_columns;
};

class Logger : public QObject
{
    Q_OBJECT
public:
    Logger(QObject *parent = 0) : QObject(parent) {}
};



















/*
class World;
class Agent;
class ChargeAgent;
class FluxAgent;

class Logger : public QObject
{
private:
    Q_OBJECT
    Q_DISABLE_COPY(Logger)

public:
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
    void saveDefectImage();
    void saveElectronImage();
    void saveHoleImage();
    void saveImage();
    void saveParameters();

    void flush();
    void report( ChargeAgent &charge );
    void report( FluxAgent &flux );

private:

    World *m_world;
    QDir m_outputdir;

    QFile m_carrierFile;
    QTextStream m_carrierStream;

    QFile m_stepFile;
    QTextStream m_stepStream;

    void beginImage( QImage &image, QPainter &painter, QColor background = QColor(Qt::white) );
    void drawSites( QPainter &painter, QList<int> &sites, QColor color = QColor(Qt::black), int layer = 0 );
    void drawCharges( QPainter &painter, QList<ChargeAgent *> &charges, QColor color = QColor(Qt::black), int layer = 0 );
    void endImage( QImage &image, QPainter &painter, QString name, int scale = 5 );
    void createFileStream( QTextStream &stream, QFile &file, QString fileName );
    void setStreamParameters( QTextStream &stream );
    QString generateFileName( QString identifier, QString extension = QString("dat") );
};*/
}
#endif // LOGGER_H
