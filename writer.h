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
    void writeVMDInitFile();
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

//! A class to output carrier stats (lifetime and pathlength)
class ExcitonWriter : public QObject
{
    Q_OBJECT
public:
    //! Constructs the writer, has the same parameters as OutputInfo
    ExcitonWriter(World &world,
                  const QString& name,
                  QObject *parent = 0);
    void write(ChargeAgent &charge1, ChargeAgent &charge2, bool recombined = false);
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
      \param world Reference to the world object
      \param bg Background color
      \param parent parent QObject
      */
    GridImage(World &world, QColor bg = Qt::black, QObject *parent=0);

    //! Draw some sites
    /*!
      \param sites A list of integers that are site ids
        - could be the list of trap ids
        - could be the list of defect ids
      \param color The color of the points
      \param layer Which layer are we drawing? its a 2D image
      */
    void drawSites(QList<int> &sites, QColor color, int layer);

    //! Draw some sites
    /*!
      \param sites A list of ChargeAgents, which have site ids
        - could be the list of electrons
        - could be the list of holes
      \param color The color of the points
      \param layer Which layer are we drawing? its a 2D image
      */
    void drawCharges(QList<ChargeAgent *> &charges,QColor color, int layer);

    //! Save the image to a file
    /*!
      \param name A file name that is passed to a OutputInfo object, the output is assummed png
      \param scale Multiply the image by some scale, increasing the resolution
      */
    void save(QString name, int scale=3);
private:
    //! The painter that paints the image
    QPainter m_painter;

    //! The image we draw onto
    QImage m_image;

    //! Reference to the world object
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
    //! Does nothing \see LoggerOn::LoggerOn
    Logger(World &world, QObject *parent = 0) : QObject(parent) {}

    //! Does nothing \see LoggerOn::saveTrapImage
    virtual void saveTrapImage(const QString& name = "%stub-traps.png") {}

    //! Does nothing \see LoggerOn::saveHoleImage
    virtual void saveHoleImage(const QString& name = "%stub-%step-holes.png") {}

    //! Does nothing \see LoggerOn::saveElectronImage
    virtual void saveElectronImage(const QString& name = "%stub-%step-electrons.png") {}

    //! Does nothing \see LoggerOn::saveElectronImage
    virtual void saveCarriersImage(const QString& name = "%stub-%step-carriers.png") {}

    //! Does nothing \see LoggerOn::saveDefectImage
    virtual void saveDefectImage(const QString& name = "%stub-defects.png") {}

    //! Does nothing \see LoggerOn::saveImage
    virtual void saveImage(const QString& name = "%stub-%step-all.png") {}

    //! Does nothing \see LoggerOn::saveGridPotential
    virtual void saveGridPotential(const QString& name = "%stub.grid") {}

    //! Does nothing \see LoggerOn::saveCoulombEnergy
    virtual void saveCoulombEnergy(const QString& name = "%stub-%step.coulomb") {}

    //! Does nothing \see LoggerOn::reportFluxStream
    virtual void reportFluxStream() {}

    //! Does nothing \see LoggerOn::reportXYZStream
    virtual void reportXYZStream() {}

    //! Does nothing \see LoggerOn::reportCarrier
    virtual void reportCarrier(ChargeAgent &charge) {}

    //! Does nothing \see LoggerOn::reportExciton
    virtual void reportExciton(ChargeAgent &charge1, ChargeAgent &charge2, bool recombined = false) {}
};

class LoggerOn : public Logger
{
public:
    LoggerOn(World &world, QObject *parent = 0);

    //! Save an image of trap sites as png
    virtual void saveTrapImage(const QString& name = "%stub-traps.png");

    //! Save an image of holes (at the current step) as png
    virtual void saveHoleImage(const QString& name = "%stub-%step-holes.png");

    //! Save an image of electrons (at the current step) as png
    virtual void saveElectronImage(const QString& name = "%stub-%step-electrons.png");

    //! Save an image of holes \b and electrons (at the current step) as png
    virtual void saveCarriersImage(const QString& name = "%stub-%step-carriers.png");

    //! Save an image of defects as png
    virtual void saveDefectImage(const QString& name = "%stub-defects.png");

    //! Save an image of electrons, holes, defects, and traps (at current step) as png
    virtual void saveImage(const QString& name = "%stub-%step-all.png");

    //! Output the grid potential as (x, y, z, v) to a file
    virtual void saveGridPotential(const QString& name = "%stub.grid");

    //! Output the Coulomb potential as (x, y, z, v) to a file; \b requires the use of the \b GPU
    virtual void saveCoulombEnergy(const QString& name = "%stub-%step.coulomb");

    //! Output information about Sources and Drains (at the current step) to the main output file
    virtual void reportFluxStream();

    //! Output xyz information (at the current step) to the xyz file
    virtual void reportXYZStream();

    //! Output carrier information (for example pathlength) to the carrier file
    virtual void reportCarrier(ChargeAgent &charge);

    //! Output carrier information (for example pathlength) on two carriers at once to the exciton file
    virtual void reportExciton(ChargeAgent &charge1, ChargeAgent &charge2, bool recombined = false);

protected:

    //! Open the various output streams if they are turned on
    virtual void initialize();

    //! Reference to world
    World &m_world;

    //! Writer in charge of writing xyz files
    XYZWriter *m_xyzWriter;

    //! Writer in charge of writing source & drain information
    FluxWriter *m_fluxWriter;

    //! Writer in charge of writing carrier information
    CarrierWriter *m_carrierWriter;

    //! Writer in charge of writing multiple carrier's information (excitons)
    ExcitonWriter *m_excitonWriter;
};

}
#endif // WRITER_H
