#include "inputparser.h"
#include "chargeagent.h"
#include "sourceagent.h"
#include "drainagent.h"
#include "potential.h"
#include "cubicgrid.h"
#include "logger.h"
#include "world.h"
#include "rand.h"

using namespace Langmuir;

Logger::Logger(World * world, QObject *parent): QObject(parent), m_world(world)
{
    m_outputdir = QDir(m_world->parameters()->outputPath);
    if(!m_outputdir.exists()) { qFatal("%s does not exist", qPrintable(m_outputdir.path()));}

    if(m_world->parameters()->outputStats)
    {
        m_carrierFile.setFileName(m_outputdir.absoluteFilePath("carrier-lifetime-pathlength.dat"));
        if(! m_carrierFile.open(QIODevice::Text|QIODevice::WriteOnly))
        {
            qFatal("error opening stat file");
        }
        m_carrierStream.setDevice(&m_carrierFile);
        m_carrierStream.setRealNumberPrecision(m_world->parameters()->outputPrecision);
        m_carrierStream.setFieldWidth(m_world->parameters()->outputWidth);
        m_carrierStream << scientific;
    }
}

Logger::~Logger()
{
    if(m_world->parameters()->outputStats){ carrierStreamFlush(); }
}

void Logger::saveCarrierIDsToFile(QString name)
{
    QFile out(name);
    int w = m_world->parameters()->outputWidth;
    if(!(out.open(QIODevice::WriteOnly | QIODevice::Text)) )
    {
        qFatal("can not open file %s", qPrintable(name));
    }
    for(int i = 0; i < m_world->electrons()->size(); i++)
    {
        int s = m_world->electrons()->at(i)->site(false);
        int x = m_world->electronGrid()->getIndexX(s);
        int y = m_world->electronGrid()->getIndexY(s);
        int z = m_world->electronGrid()->getIndexZ(s);
        out.write(qPrintable(QString("%1 %2 %3 %4\n").arg(x, w).arg(y, w).arg(z, w).arg(s, w)) );
    }
}

void Logger::saveTrapIDsToFile(QString name)
{
    if(m_world->trapSiteIDs()->size()== 0)return;
    QFile out(name);
    int w = m_world->parameters()->outputWidth;
    if(!(out.open(QIODevice::WriteOnly | QIODevice::Text)) )
    {
        qFatal("can not open file %s", qPrintable(name));
    }
    for(int i = 0; i < m_world->trapSiteIDs()->size(); i++)
    {
        int s = m_world->trapSiteIDs()->at(i);
        int x = m_world->electronGrid()->getIndexX(s);
        int y = m_world->electronGrid()->getIndexY(s);
        int z = m_world->electronGrid()->getIndexZ(s);
        out.write(qPrintable(QString("%1 %2 %3 %4\n").arg(x, w).arg(y, w).arg(z, w).arg(s, w)) );
    }
    out.close();
}

void Logger::saveDefectIDsToFile(QString name)
{
    if(m_world->defectSiteIDs()->size()== 0)return;
    QFile out(name);
    int w = m_world->parameters()->outputWidth;
    if(!(out.open(QIODevice::WriteOnly | QIODevice::Text)) )
    {
        qFatal("can not open file %s", qPrintable(name));
    }
    for(int i = 0; i < m_world->defectSiteIDs()->size(); i++)
    {
        int s = m_world->defectSiteIDs()->at(i);
        int x = m_world->electronGrid()->getIndexX(s);
        int y = m_world->electronGrid()->getIndexY(s);
        int z = m_world->electronGrid()->getIndexZ(s);
        out.write(qPrintable(QString("%1 %2 %3 %4\n").arg(x, w).arg(y, w).arg(z, w).arg(s, w)) );
    }
    out.close();
}

void Logger::saveFieldEnergyToFile(QString name)
{
    QFile out(name);
    int p = m_world->parameters()->outputPrecision;
    int w = m_world->parameters()->outputWidth;
    if(!(out.open(QIODevice::WriteOnly | QIODevice::Text)) )
    {
        qFatal("can not open file %s", qPrintable(name));
    }
    for(int i = 0; i < m_world->electronGrid()->xSize(); i++)
    {
        for(int j = 0; j < m_world->electronGrid()->ySize(); j++)
        {
            for(int k = 0; k < m_world->electronGrid()->zSize(); k++)
            {
                int s = m_world->electronGrid()->getIndexS(i, j, k);
                out.write(qPrintable(QString("%1 %2 %3 %4\n").arg(i, w).arg(j, w).arg(k, w).arg(m_world->electronGrid()->potential(s), w, 'e', p)) );
            }
        }
    }
    out.close();
}

void Logger::saveCoulombEnergyToFile(QString name)
{
    /*
    QFile out(name);
    int p = m_world->parameters()->outputPrecision;
    int w = m_world->parameters()->outputWidth;
    if(!(out.open(QIODevice::WriteOnly | QIODevice::Text)) )
    {
        qFatal("can not open file %s", qPrintable(name));
    }
    for(int i = 0; i < m_world->grid()->width(); i++)
    {
        for(int j = 0; j < m_world->grid()->height(); j++)
        {
            for(int k = 0; k < m_world->grid()->depth(); k++)
            {
                int s = m_world->grid()->getIndex(i, j, k);
                out.write(qPrintable(QString("%1 %2 %3 %4\n").arg(i, w).arg(j, w).arg(k, w).arg(m_oHost[s]*m_world->parameters()->electrostaticPrefactor, w, 'e', p)) );
            }
        }
    }
    out.close();
*/
}

void Logger::saveTrapImageToFile(QString name)
{
    //Create an empty image of the appropriate size
    QImage img(m_world->parameters()->xSize, m_world->parameters()->ySize, QImage::Format_ARGB32_Premultiplied);
    img.fill(0);
    //Create a painter to draw the image
    QPainter painter;
    painter.begin(&img);
    //Move the origin from top left to bottom right
    painter.scale(1.0, -1.0);
    painter.translate(0.0, -m_world->parameters()->ySize);
    //Resize the painter window to the grid dimensions
    painter.setWindow(QRect(0, 0, m_world->parameters()->xSize, m_world->parameters()->ySize));
    //Set the background to be white
    painter.fillRect(QRect(0, 0, m_world->parameters()->xSize, m_world->parameters()->ySize), Qt::white);
    //Set the trap color to a light gray
    painter.setPen(QColor(255, 0, 0, 255));
    //Draw the traps as dots
    for(int i = 0; i < m_world->trapSiteIDs()->size(); i++)
    {
        //Index of a trap site
        int ndx = m_world->trapSiteIDs()->at(i);
        //If this trap is in the Layer we are drawing...
        if(m_world->electronGrid()->getIndexZ(ndx)== 0)
        {
            //Draw the trap as a point at(x, y)=(col, row)
            painter.drawPoint(QPoint(m_world->electronGrid()->getIndexX(ndx), m_world->electronGrid()->getIndexY(ndx)) );
        }
    }
    painter.end();
    //return the new image(its not copied on return because of how Qt stores images)
    img = img.scaled(5*img.width(), 5*img.height(), Qt::KeepAspectRatioByExpanding);
    img.save(name, "png", 100);
}

void Logger::carrierStreamFlush()
{
    m_carrierStream.flush();
}

void Logger::carrierReportLifetimeAndPathlength(int i, int tick)
{
    m_carrierStream << QString("%1 %2 %3\n")
                       .arg(tick, m_world->parameters()->outputWidth)
                       .arg(m_world->electrons()->at(i)->lifetime(), m_world->parameters()->outputWidth)
                       .arg(m_world->electrons()->at(i)->pathlength(), m_world->parameters()->outputWidth);
}
