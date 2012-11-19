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
    //! constructs the writer, has the same parameters as OutputInfo
    XYZWriter(World &world,
              const QString& name,
              QObject *parent = 0);

    //! Write XYZ of the current step to the stream
    void write();
protected:
    //! reference to the world object
    World &m_world;

    //! output file stream
    OutputStream m_stream;

    //! write a VMD script useful for opening the XYZ file
    void writeVMDInitFile();
};

//! A class to output source and drain info
class FluxWriter : public QObject
{
    Q_OBJECT
public:
    //! constructs the writer, has the same parameters as OutputInfo
    FluxWriter(World &world,
               const QString& name,
               QObject *parent = 0);

    //! write the flux statistics of the current step to the stream
    void write();
protected:
    //! reference to the world object
    World &m_world;

    //! output file stream
    OutputStream m_stream;
};

//! A class to output carrier stats (lifetime and pathlength)
class CarrierWriter : public QObject
{
    Q_OBJECT
public:
    //! constructs the writer, has the same parameters as OutputInfo
    CarrierWriter(World &world,
                  const QString& name,
                  QObject *parent = 0);

    //! write the charge carrier statistics to the stream
    void write(ChargeAgent &charge);
protected:
    //! reference to the world object
    World &m_world;

    //! output file stream
    OutputStream m_stream;
};

//! A class to output exciton stats (lifetime and pathlength)
class ExcitonWriter : public QObject
{
    Q_OBJECT
public:
    //! constructs the writer, has the same parameters as OutputInfo
    ExcitonWriter(World &world,
                  const QString& name,
                  QObject *parent = 0);

    //! write the exciton statistics to the stream
    void write(ChargeAgent &charge1, ChargeAgent &charge2, bool recombined = false);
protected:
    //! reference to the world object
    World &m_world;

    //! output file stream
    OutputStream m_stream;
};

//! A class to draw images of the grid
class GridImage : public QObject
{
    Q_OBJECT
public:
    //! create the image and painter, setting the background and size
    /*!
      \param world Reference to the world object
      \param bg Background color
      \param parent parent QObject
      */
    GridImage(World &world, QColor bg = Qt::black, QObject *parent=0);

    //! draw some sites
    /*!
      \param sites A list of integers that are site ids
        - could be the list of trap ids
        - could be the list of defect ids
      \param color The color of the points
      \param layer Which layer are we drawing? its a 2D image
      */
    void drawSites(QList<int> &sites, QColor color, int layer);

    //! draw some sites
    /*!
      \param charges A list of ChargeAgents, which have site ids
        - could be the list of electrons
        - could be the list of holes
      \param color The color of the points
      \param layer Which layer are we drawing? its a 2D image
      */
    void drawCharges(QList<ChargeAgent *> &charges, QColor color, int layer);

    //! save the image to a file
    /*!
      \param name A file name that is passed to a OutputInfo object, the output is assummed png
      \param scale Multiply the image by some scale, increasing the resolution
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

//! A class that organizes output (actually, this base class does nothing - LoggerOn is used when the output is on)
/*!
  This version of the class is a base class that actually does nothing when any
  of its function are called - used to turn the output off.
  */
class Logger : public QObject
{
    Q_OBJECT
public:
    //! does nothing \see LoggerOn::LoggerOn
    Logger(World &world, QObject *parent = 0) : QObject(parent) {}

    //! does nothing \see LoggerOn::saveTrapImage
    virtual void saveTrapImage(const QString& name = "%stub-traps.png") {}

    //! does nothing \see LoggerOn::saveHoleImage
    virtual void saveHoleImage(const QString& name = "%stub-%step-holes.png") {}

    //! does nothing \see LoggerOn::saveElectronImage
    virtual void saveElectronImage(const QString& name = "%stub-%step-electrons.png") {}

    //! does nothing \see LoggerOn::saveElectronImage
    virtual void saveCarriersImage(const QString& name = "%stub-%step-carriers.png") {}

    //! does nothing \see LoggerOn::saveDefectImage
    virtual void saveDefectImage(const QString& name = "%stub-defects.png") {}

    //! does nothing \see LoggerOn::saveImage
    virtual void saveImage(const QString& name = "%stub-%step-all.png") {}

    //! does nothing \see LoggerOn::saveGridPotential
    virtual void saveGridPotential(const QString& name = "%stub.grid") {}

    //! does nothing \see LoggerOn::saveCoulombEnergy
    virtual void saveCoulombEnergy(const QString& name = "%stub-%step.coulomb") {}

    //! does nothing \see LoggerOn::reportFluxStream
    virtual void reportFluxStream() {}

    //! does nothing \see LoggerOn::reportXYZStream
    virtual void reportXYZStream() {}

    //! does nothing \see LoggerOn::reportCarrier
    virtual void reportCarrier(ChargeAgent &charge) {}

    //! does nothing \see LoggerOn::reportExciton
    virtual void reportExciton(ChargeAgent &charge1, ChargeAgent &charge2, bool recombined = false) {}
};

//! A class that organizes output (derived from Logger)
class LoggerOn : public Logger
{
public:
    //! create Logger
    LoggerOn(World &world, QObject *parent = 0);

    //! save an image of trap sites as png
    virtual void saveTrapImage(const QString& name = "%stub-traps.png");

    //! save an image of holes (at the current step) as png
    virtual void saveHoleImage(const QString& name = "%stub-%step-holes.png");

    //! save an image of electrons (at the current step) as png
    virtual void saveElectronImage(const QString& name = "%stub-%step-electrons.png");

    //! save an image of holes \b and electrons (at the current step) as png
    virtual void saveCarriersImage(const QString& name = "%stub-%step-carriers.png");

    //! save an image of defects as png
    virtual void saveDefectImage(const QString& name = "%stub-defects.png");

    //! save an image of electrons, holes, defects, and traps (at current step) as png
    virtual void saveImage(const QString& name = "%stub-%step-all.png");

    //! output the grid potential as (x, y, z, v) to a file
    virtual void saveGridPotential(const QString& name = "%stub.grid");

    //! output the Coulomb potential as (x, y, z, v) to a file; \b requires the use of the \b GPU
    virtual void saveCoulombEnergy(const QString& name = "%stub-%step.coulomb");

    //! output information about Sources and Drains (at the current step) to the main output file
    virtual void reportFluxStream();

    //! output xyz information (at the current step) to the xyz file
    virtual void reportXYZStream();

    //! output carrier information (for example pathlength) to the carrier file
    virtual void reportCarrier(ChargeAgent &charge);

    //! output carrier information (for example pathlength) on two carriers at once to the exciton file
    virtual void reportExciton(ChargeAgent &charge1, ChargeAgent &charge2, bool recombined = false);

protected:
    //! open the various output streams if they are turned on
    virtual void initialize();

    //! reference to world
    World &m_world;

    //! writer in charge of writing xyz files
    XYZWriter *m_xyzWriter;

    //! writer in charge of writing source & drain information
    FluxWriter *m_fluxWriter;

    //! writer in charge of writing carrier information
    CarrierWriter *m_carrierWriter;

    //! writer in charge of writing multiple carrier's information (excitons)
    ExcitonWriter *m_excitonWriter;
};

}
#endif // WRITER_H
