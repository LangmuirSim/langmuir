#ifndef WRITER_H
#define WRITER_H

#include <QObject>
#include <QPainter>
#include <QColor>
#include <QImage>

#include "output.h"

namespace Langmuir
{

class ChargeAgent;
class FluxAgent;
class World;
class Grid;

//! A class to output xyz files
class XYZWriter : public QObject
{
    Q_OBJECT
public:
    //! Constructs the writer, has the same parameters as OutputInfo
    XYZWriter(World &world,
              const QString& name,
              QObject *parent = 0);
    void write();
protected:
    World &m_world;
    OutputStream m_stream;
};

//! A class to output source and drain info
class FluxWriter : public QObject
{
    Q_OBJECT
public:
    //! Constructs the writer, has the same parameters as OutputInfo
    FluxWriter(World &world,
               const QString& name,
               QObject *parent = 0);
    void write();
protected:
    World &m_world;
    OutputStream m_stream;
};

//! A class to output carrier stats (lifetime and pathlength)
class CarrierWriter : public QObject
{
    Q_OBJECT
public:
    //! Constructs the writer, has the same parameters as OutputInfo
    CarrierWriter(World &world,
                  const QString& name,
                  QObject *parent = 0);
    void write(ChargeAgent &charge);
protected:
    World &m_world;
    OutputStream m_stream;
};

//! A class to draw images of the grid
class GridImage : public QObject
{
    Q_OBJECT
public:
    //! Create the image and painter, setting the background and size
    /*!
      \param world reference to the world object
      \param bg background color
      \param parent the parent of this QObject
      */
    GridImage(World &world, QColor bg = Qt::black, QObject *parent=0);

    //! Draw some sites
    /*!
      \param sites a list of integers that are site ids
        - could be the list of trap ids
        - could be the list of defect ids
      \param color the color of the points
      \param layer which layer are we drawing? its a 2D image
      */
    void drawSites(QList<int> &sites, QColor color, int layer);

    //! Draw some sites
    /*!
      \param sites a list of ChargeAgents, which have site ids
        - could be the list of electrons
        - could be the list of holes
      \param color the color of the points
      \param layer which layer are we drawing? its a 2D image
      */
    void drawCharges(QList<ChargeAgent *> &charges,QColor color, int layer);

    //! save the image to a file
    /*!
      \param name a file name that is passed to a OutputInfo object, the output is assummed png
      \param scale multiple the image by some scale, increasing the resolution
      */
    void save(QString name, int scale=3);
private:
    //! the painter that paints the image
    QPainter m_painter;

    //! the image we draw onto
    QImage m_image;

    //! reference to the world object
    World &m_world;
};

//! A class that organizes output
/*!
  This version of the class is a Base that actually does nothing when any
  of its function are called - used to turn the output off.
  */
class Logger : public QObject
{
    Q_OBJECT
public:
    //! does nothing \see LoggerOn::LoggerOn
    Logger(World &world, QObject *parent = 0) : QObject(parent) {}
    //! does nothing \see LoggerOn::saveTrapImage
    virtual void saveTrapImage(const QString& name = "%path/%stub-%sim-traps.png") {}
    //! does nothing \see LoggerOn::saveHoleImage
    virtual void saveHoleImage(const QString& name = "%path/%stub-%sim-%step-holes.png") {}
    //! does nothing \see LoggerOn::saveElectronImage
    virtual void saveElectronImage(const QString& name = "%path/%stub-%sim-%step-electrons.png") {}
    //! does nothing \see LoggerOn::saveDefectImage
    virtual void saveDefectImage(const QString& name = "%path/%stub-%sim-defects.png") {}
    //! does nothing \see LoggerOn::saveImage
    virtual void saveImage(const QString& name = "%path/%stub-%sim-%step-all.png") {}
    //! does nothing \see LoggerOn::saveGridPotential
    virtual void saveGridPotential(const QString& name = "%path/%stub-%sim-grid.dat") {}
    //! does nothing \see LoggerOn::saveCoulombEnergy
    virtual void saveCoulombEnergy(const QString& name = "%path/%stub-%sim-%step-coulomb.dat") {}
    //! does nothing \see LoggerOn::reportFluxStream
    virtual void reportFluxStream() {}
    //! does nothing \see LoggerOn::reportXYZStream
    virtual void reportXYZStream() {}
    //! does nothing \see LoggerOn::reportCarrier
    virtual void reportCarrier(ChargeAgent &charge) {}
};

class LoggerOn : public Logger
{
public:
    LoggerOn(World &world, QObject *parent = 0);

    virtual void saveTrapImage(const QString& name = "%path/%stub-%sim-traps.png");
    virtual void saveHoleImage(const QString& name = "%path/%stub-%sim-%step-holes.png");
    virtual void saveElectronImage(const QString& name = "%path/%stub-%sim-%step-electrons.png");
    virtual void saveDefectImage(const QString& name = "%path/%stub-%sim-defects.png");
    virtual void saveImage(const QString& name = "%path/%stub-%sim-%step-all.png");

    virtual void saveGridPotential(const QString& name = "%path/%stub-%sim-grid.dat");
    virtual void saveCoulombEnergy(const QString& name = "%path/%stub-%sim-%step-coulomb.dat");

    virtual void reportFluxStream();
    virtual void reportXYZStream();
    virtual void reportCarrier(ChargeAgent &charge);

protected:
    virtual void initialize();
    World &m_world;
    XYZWriter *m_xyzWriter;
    FluxWriter *m_fluxWriter;
    CarrierWriter *m_carrierWriter;
};

}
#endif // WRITER_H
