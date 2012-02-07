#include "openclhelper.h"
#include "fluxagent.h"
#include "parameters.h"
#include "chargeagent.h"
#include "cubicgrid.h"
#include "sourceagent.h"
#include "drainagent.h"
#include "logger.h"
#include "world.h"

namespace Langmuir
{

DataStream::DataStream(QString name, int width, int precision)
    : QTextStream(), m_width(width), m_precision(precision)
{
    m_file.setFileName(name);
    setDevice(&m_file);
    if (m_file.exists() || m_file.isOpen())
    {
        qFatal("can not open data file (%s); file exists or is open already",qPrintable(name));
    }
    if (!m_file.open(QIODevice::WriteOnly|QIODevice::Text))
    {
        qFatal("can not open data file (%s)",qPrintable(name));
    }
    setStreamParameters();
}

void DataStream::newline()
{
    reset();
    *this << "\n";
    setStreamParameters();
}

void DataStream::setStreamParameters()
{
    setRealNumberPrecision(m_precision);
    setRealNumberNotation(QTextStream::ScientificNotation);
    setFieldWidth(m_width);
    setFieldAlignment(QTextStream::AlignRight);
}

void DataStream::fillColumns(int count, QString fill)
{
    if ( count < 0 ) return;
    for (int i = 0; i < count; i++)
    {
        *this << fill;
    }
}

Logger::Logger(World &world, QObject *parent)
    : QObject(parent), m_world(world),
      m_fluxStream(0),
      m_carrierStream(0)
{
    m_outputdir = QDir(m_world.parameters().outputPath);

    if(!m_outputdir.exists())
    {
        qFatal("%s does not exist", qPrintable(m_outputdir.path()));
    }

    if(m_world.parameters().outputIdsOnDelete)
    {
        if (m_carrierStream) return;

        m_carrierStream = new DataStream(
                    generateFileName("onDelete","dat",true,false),
                    m_world.parameters().outputWidth,
                    m_world.parameters().outputPrecision);
        *m_carrierStream << "si"
                         << "xi"
                         << "yi"
                         << "zi"
                         << "lifetime"
                         << "pathlength"
                         << "type"
                         << "step";
        m_carrierStream->newline();
        m_carrierStream->flush();
    }

    if ( m_world.drains().size() > 0 || m_world.sources().size() > 0 )
    {
        m_fluxStream = new DataStream(
                    generateFileName("","dat",true,false),
                    m_world.parameters().outputWidth,
                    m_world.parameters().outputPrecision);

        QList<FluxAgent *>& fluxAgents = m_world.fluxes();
        DataStream& stream = *m_fluxStream;

        for (int i = 0; i < fluxAgents.size()*2 + 1; i++) { stream << i; }
        stream << fluxAgents.size()*2 + 1 << fluxAgents.size()*2 + 2;
        stream.newline();

        stream << "Agent";
        foreach(FluxAgent *flux, fluxAgents)
        {
            stream << flux->objectName();
            stream.fillColumns(1);
        }
        stream.fillColumns(2);
        stream.newline();

        stream << "Face";
        foreach(FluxAgent *flux, fluxAgents)
        {
            stream << flux->face();
            stream.fillColumns(1);
        }
        stream.fillColumns(2);
        stream.newline();

        stream << "Step";
        for(int i = 0; i < fluxAgents.size(); i++)
        {
            stream << "attempts" << "successes";
        }
        stream << "electrons" << "holes";
        stream.newline();
        stream.flush();
    }
}

Logger::~Logger()
{
    if (m_fluxStream) { m_fluxStream->flush(); delete m_fluxStream; }
    if (m_carrierStream) { m_carrierStream->flush(); delete m_carrierStream; }
}

void Logger::report( ChargeAgent &charge )
{
    Grid &grid = m_world.electronGrid();
    int si = charge.getCurrentSite();
    *m_carrierStream << si
                     << grid.getIndexX(si)
                     << grid.getIndexY(si)
                     << grid.getIndexZ(si)
                     << charge.lifetime()
                     << charge.pathlength()
                     << charge.getType()
                     << m_world.parameters().currentStep;
    m_carrierStream->newline();
}


void Logger::saveElectronIDs()
{
    QList< ChargeAgent * > &chargeAgents = m_world.electrons();
    Grid &grid = m_world.electronGrid();

    if(chargeAgents.size()==0)return;

    QString fname = generateFileName("electronIDs");
    DataStream stream(fname,
                m_world.parameters().outputWidth,
                m_world.parameters().outputPrecision);

    stream << "si"
           << "xi"
           << "yi"
           << "zi"
           << "lifetime"
           << "pathlength";
    stream.newline();

    for ( int i = 0; i < chargeAgents.size(); i++ )
    {
        int si = chargeAgents[i]->getCurrentSite();
        stream << si
               << grid.getIndexX(si)
               << grid.getIndexY(si)
               << grid.getIndexZ(si)
               << chargeAgents[i]->lifetime()
               << chargeAgents[i]->pathlength();
        stream.newline();
    }
    stream.flush();
}

void Logger::saveHoleIDs()
{
    QList< ChargeAgent * > &chargeAgents = m_world.holes();
    Grid &grid = m_world.holeGrid();

    if(chargeAgents.size()==0)return;

    QString fname = generateFileName("holeIDs");
    DataStream stream(fname,
                m_world.parameters().outputWidth,
                m_world.parameters().outputPrecision);

    stream << "si"
           << "xi"
           << "yi"
           << "zi"
           << "lifetime"
           << "pathlength";
    stream.newline();

    for ( int i = 0; i < chargeAgents.size(); i++ )
    {
        int si = chargeAgents[i]->getCurrentSite();
        stream << si
               << grid.getIndexX(si)
               << grid.getIndexY(si)
               << grid.getIndexZ(si)
               << chargeAgents[i]->lifetime()
               << chargeAgents[i]->pathlength();
        stream.newline();
    }
    stream.flush();
}

void Logger::saveTrapIDs()
{
    QList<int> &siteIDs = m_world.trapSiteIDs();
    Grid &grid = m_world.electronGrid();

    if(siteIDs.size()==0)return;

    QString fname = generateFileName("trapIDs");
    DataStream stream(fname,
                m_world.parameters().outputWidth,
                m_world.parameters().outputPrecision);

    stream << "si"
           << "xi"
           << "yi"
           << "zi";
    stream.newline();

    for ( int i = 0; i < siteIDs.size(); i++ )
    {
        int si = siteIDs[i];
        stream << si
               << grid.getIndexX(si)
               << grid.getIndexY(si)
               << grid.getIndexZ(si);
        stream.newline();
    }
    stream.flush();
}

void Logger::saveDefectIDs()
{
    QList<int> &siteIDs = m_world.defectSiteIDs();
    Grid &grid = m_world.electronGrid();

    if(siteIDs.size()==0)return;

    QString fname = generateFileName("defectIDs");
    DataStream stream(fname,
                m_world.parameters().outputWidth,
                m_world.parameters().outputPrecision);

    stream << "si"
           << "xi"
           << "yi"
           << "zi";
    stream.newline();

    for ( int i = 0; i < siteIDs.size(); i++ )
    {
        int si = siteIDs[i];
        stream << si
               << grid.getIndexX(si)
               << grid.getIndexY(si)
               << grid.getIndexZ(si);
        stream.newline();
    }
    stream.flush();
}

void Logger::saveElectronGridPotential()
{
    Grid &grid = m_world.electronGrid();

    QString fname = generateFileName("eGrid");
    DataStream stream(fname,
                m_world.parameters().outputWidth,
                m_world.parameters().outputPrecision);

    stream << "si"
           << "xi"
           << "yi"
           << "zi"
           << "vi";
    stream.newline();

    for(int i = 0; i < m_world.electronGrid().xSize(); i++)
    {
        for(int j = 0; j < m_world.electronGrid().ySize(); j++)
        {
            for(int k = 0; k < m_world.electronGrid().zSize(); k++)
            {
                int si = grid.getIndexS(i,j,k);
                stream << si
                       << i
                       << j
                       << k
                       << grid.potential(si);
                stream.newline();
            }
        }
    }
    stream.flush();
}

void Logger::saveHoleGridPotential()
{
    Grid &grid = m_world.holeGrid();

    QString fname = generateFileName("hGrid");
    DataStream stream(fname,
                m_world.parameters().outputWidth,
                m_world.parameters().outputPrecision);

    stream << "si"
           << "xi"
           << "yi"
           << "zi"
           << "vi";
    stream.newline();

    for(int i = 0; i < m_world.electronGrid().xSize(); i++)
    {
        for(int j = 0; j < m_world.electronGrid().ySize(); j++)
        {
            for(int k = 0; k < m_world.electronGrid().zSize(); k++)
            {
                int si = grid.getIndexS(i,j,k);
                stream << si
                       << i
                       << j
                       << k
                       << grid.potential(si);
                stream.newline();
            }
        }
    }
    stream.flush();
}

void Logger::saveCoulombEnergy()
{
    Grid &grid = m_world.electronGrid();
    OpenClHelper &openCL = m_world.opencl();

    QString fname = generateFileName("coulomb");
    DataStream stream(fname,
                m_world.parameters().outputWidth,
                m_world.parameters().outputPrecision);

    stream << "si"
           << "xi"
           << "yi"
           << "zi"
           << "vi";
    stream.newline();

    for(int i = 0; i < m_world.electronGrid().xSize(); i++)
    {
        for(int j = 0; j < m_world.electronGrid().ySize(); j++)
        {
            for(int k = 0; k < m_world.electronGrid().zSize(); k++)
            {
                int si = grid.getIndexS(i,j,k);
                stream << si
                       << i
                       << j
                       << k
                       << openCL.getOutputHost(si);
                stream.newline();
            }
        }
    }
    stream.flush();
}

QString Logger::generateFileName( QString identifier, QString extension, bool sim, bool step )
{
    int w1 = int(log10(m_world.parameters().simulationSteps))+1;
    int w2 = int(log10(m_world.parameters().iterationsReal))+1;
    QString name = QString("%1").arg(m_world.parameters().outputStub);
    if (sim) name += QString("-%1").arg(m_world.parameters().currentSimulation,w1,10,QLatin1Char('0'));
    if (step) name += QString("-%1").arg(m_world.parameters().currentStep,w2,10,QLatin1Char('0'));
    if (!(identifier.isEmpty())) name += QString("-%1").arg(identifier);
    if (!(extension.isEmpty())) name += QString(".%1").arg(extension);
    return m_outputdir.absoluteFilePath(name);
}

GridImage::GridImage(World &world, QColor bg, QObject *parent)
    : QObject(parent), m_world(world)
{
    int xsize = m_world.parameters().gridX;
    int ysize = m_world.parameters().gridY;
    m_image = QImage(xsize,ysize,QImage::Format_ARGB32_Premultiplied);
    m_image.fill(0);
    m_painter.begin(&m_image);
    m_painter.scale(1.0,-1.0);
    m_painter.translate(0.0,-ysize);
    m_painter.setWindow(QRect(0,0,xsize,ysize));
    m_painter.fillRect(QRect(0,0,xsize,ysize),bg);
}

void GridImage::save(QString name, int scale)
{
    m_painter.end();
    m_image = m_image.scaled(scale*m_image.width(),scale*m_image.height(),Qt::KeepAspectRatioByExpanding);
    m_image.save(name,"png",100);
}

void GridImage::drawSites( QList<int> &sites, QColor color, int layer )
{
    if (sites.size()<=0)return;
    Grid &grid = m_world.electronGrid();
    m_painter.setPen(color);
    for(int i = 0; i < sites.size(); i++)
    {
        int ndx = sites[i];
        if(grid.getIndexZ(ndx)== layer)
        {
            m_painter.drawPoint(QPoint(grid.getIndexX(ndx),grid.getIndexY(ndx)));
        }
    }
}

void GridImage::drawCharges( QList<ChargeAgent *> &charges,QColor color, int layer )
{
    if (charges.size()<=0)return;
    Grid &grid = m_world.electronGrid();
    m_painter.setPen(color);
    for(int i = 0; i < charges.size(); i++)
    {
        int ndx = charges[i]->getCurrentSite();
        if(grid.getIndexZ(ndx)== layer)
        {
            m_painter.drawPoint(QPoint(grid.getIndexX(ndx),grid.getIndexY(ndx)));
        }
    }
}

void Logger::saveTrapImage()
{
    QList<int>& siteIDs = m_world.trapSiteIDs();
    if(siteIDs.size()==0)return;
    GridImage image(m_world,Qt::white,this);
    image.drawSites(siteIDs,Qt::green,0);
    image.save(generateFileName("traps","png"));
}

void Logger::saveDefectImage()
{
    QList<int>& siteIDs = m_world.defectSiteIDs();
    if(siteIDs.size()==0)return;
    GridImage image(m_world,Qt::white,this);
    image.drawSites(siteIDs,Qt::cyan,0);
    image.save(generateFileName("defects","png"));
}

void Logger::saveElectronImage()
{
    QList<ChargeAgent*>& chargeIDs = m_world.electrons();
    if(chargeIDs.size()==0)return;
    GridImage image(m_world,Qt::white,this);
    image.drawCharges(chargeIDs,Qt::red,0);
    image.save(generateFileName("electrons","png"));
}

void Logger::saveHoleImage()
{
    QList<ChargeAgent*>& chargeIDs = m_world.holes();
    if(chargeIDs.size()==0)return;
    GridImage image(m_world,Qt::white,this);
    image.drawCharges(chargeIDs,Qt::blue,0);
    image.save(generateFileName("holes","png"));
}

void Logger::saveImage()
{
    QList<int>& traps = m_world.trapSiteIDs();
    QList<int>& defects = m_world.defectSiteIDs();
    QList<ChargeAgent*>& electrons = m_world.electrons();
    QList<ChargeAgent*>& holes = m_world.holes();
    GridImage image(m_world,Qt::white,this);
    if (traps.size()>0) { image.drawSites(traps,Qt::green,0); }
    if (defects.size()>0) { image.drawSites(defects,Qt::cyan,0); }
    if (electrons.size()>0) { image.drawCharges(electrons,Qt::red,0); }
    if (holes.size()>0) { image.drawCharges(holes,Qt::blue,0); }
    image.save(generateFileName("all","png"));
}

void Logger::reportFluxStream()
{
    DataStream& stream = *m_fluxStream;
    QList<FluxAgent *>& fluxAgents =  m_world.fluxes();
    stream << m_world.parameters().currentStep;
    foreach(FluxAgent *flux, fluxAgents)
    {
        stream << flux->attempts() << flux->successes();
    }
    stream << m_world.numElectronAgents() << m_world.numHoleAgents();
    stream.newline();
    stream.flush();
}

}
