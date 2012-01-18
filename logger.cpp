#include "openclhelper.h"
#include "inputparser.h"
#include "chargeagent.h"
#include "sourceagent.h"
#include "drainagent.h"
#include "potential.h"
#include "cubicgrid.h"
#include "logger.h"
#include "world.h"
#include "rand.h"

namespace Langmuir
{

Logger::Logger(World * world, QObject *parent): QObject(parent), m_world(world)
{
    m_outputdir = QDir(m_world->parameters()->outputPath);
    if(!m_outputdir.exists()) { qFatal("%s does not exist", qPrintable(m_outputdir.path()));}
    m_stub = QString("%1_%2").arg(m_world->parameters()->outputStub).arg(m_world->parameters()->currentSimulation);
    m_carrierFile.setFileName( m_outputdir.absoluteFilePath(QString("%1_on_delete.dat").arg(m_stub)));
    m_carrierStream.setDevice(&m_carrierFile);
    m_carrierStream.setRealNumberPrecision(m_world->parameters()->outputPrecision);
    m_carrierStream.setFieldWidth(m_world->parameters()->outputWidth);
    m_carrierStream.setRealNumberNotation(QTextStream::ScientificNotation);
    m_carrierStream.setFieldAlignment(QTextStream::AlignRight);
    m_carrierStream << scientific;
    m_carrierStream << "step";
    ChargeAgent::titleString(m_carrierStream,*m_world->parameters()); m_carrierStream << "\n";
    if ( m_world->parameters()->outputOnDelete )
    {
        if(!m_carrierFile.open(QIODevice::Text|QIODevice::WriteOnly))
        {
            qFatal("error opening on_delete file");
        }
    }
}

void Logger::flush()
{

}

void Logger::report( ChargeAgent &charge )
{
    m_carrierStream << m_world->parameters()->currentStep << charge << "\n";
}

Logger::~Logger()
{
    flush();
}

void Logger::saveElectronIDs()
{
    QFile file;
    QString name = m_outputdir.absoluteFilePath(QString("%1_%2_electron_ids.dat").arg(m_stub).arg(m_world->parameters()->currentStep));
    file.setFileName(name);
    QTextStream stream;
    stream.setDevice(&file);
    stream.setRealNumberPrecision(m_world->parameters()->outputPrecision);
    stream.setFieldWidth(m_world->parameters()->outputWidth);
    stream.setRealNumberNotation(QTextStream::ScientificNotation);
    stream.setFieldAlignment(QTextStream::AlignRight);
    if(!(file.open(QIODevice::WriteOnly | QIODevice::Text)) )
    {
        qFatal("can not open file %s", qPrintable(name));
    }
    stream << "step";
    ChargeAgent::titleString(stream,*m_world->parameters()); stream << "\n";
    for(int i = 0; i < m_world->electrons()->size(); i++)
    {
        stream << m_world->parameters()->currentStep << *m_world->electrons()->at(i) << "\n";
    }
}

void Logger::saveHoleIDs()
{
    QFile file;
    QString name = m_outputdir.absoluteFilePath(QString("%1_%2_hole_ids.dat").arg(m_stub).arg(m_world->parameters()->currentStep));
    file.setFileName(name);
    QTextStream stream;
    stream.setDevice(&file);
    stream.setRealNumberPrecision(m_world->parameters()->outputPrecision);
    stream.setFieldWidth(m_world->parameters()->outputWidth);
    stream.setRealNumberNotation(QTextStream::ScientificNotation);
    stream.setFieldAlignment(QTextStream::AlignRight);
    if(!(file.open(QIODevice::WriteOnly | QIODevice::Text)) )
    {
        qFatal("can not open file %s", qPrintable(name));
    }
    stream << "step";
    ChargeAgent::titleString(stream,*m_world->parameters()); stream << "\n";
    for(int i = 0; i < m_world->holes()->size(); i++)
    {
        stream << m_world->parameters()->currentStep << *m_world->holes()->at(i) << "\n";
    }
}

void Logger::saveTrapIDs()
{
    if(m_world->trapSiteIDs()->size()== 0)return;
    QFile file;
    QString name = m_outputdir.absoluteFilePath(QString("%1_%2_trap_ids.dat").arg(m_stub).arg(m_world->parameters()->currentStep));
    file.setFileName(name);
    QTextStream stream;
    stream.setDevice(&file);
    stream.setRealNumberPrecision(m_world->parameters()->outputPrecision);
    stream.setFieldWidth(m_world->parameters()->outputWidth);
    stream.setRealNumberNotation(QTextStream::ScientificNotation);
    stream.setFieldAlignment(QTextStream::AlignRight);
    if(!(file.open(QIODevice::WriteOnly | QIODevice::Text)) )
    {
        qFatal("can not open file %s", qPrintable(name));
    }
    stream << "step" << "s" << "x" << "y" << "z" << "\n";
    for(int i = 0; i < m_world->trapSiteIDs()->size(); i++)
    {
        int s = m_world->trapSiteIDs()->at(i);
        int x = m_world->electronGrid()->getIndexX(s);
        int y = m_world->electronGrid()->getIndexY(s);
        int z = m_world->electronGrid()->getIndexZ(s);
        stream << m_world->parameters()->currentStep << s << x << y << z << "\n";
    }
    stream.flush();
}

void Logger::saveDefectIDs()
{
    if(m_world->defectSiteIDs()->size()== 0)return;
    QFile file;
    QString name = m_outputdir.absoluteFilePath(QString("%1_%2_defect_ids.dat").arg(m_stub).arg(m_world->parameters()->currentStep));
    file.setFileName(name);
    QTextStream stream;
    stream.setDevice(&file);
    stream.setRealNumberPrecision(m_world->parameters()->outputPrecision);
    stream.setFieldWidth(m_world->parameters()->outputWidth);
    stream.setRealNumberNotation(QTextStream::ScientificNotation);
    stream.setFieldAlignment(QTextStream::AlignRight);
    if(!(file.open(QIODevice::WriteOnly | QIODevice::Text)) )
    {
        qFatal("can not open file %s", qPrintable(name));
    }
    stream << "step" << "s" << "x" << "y" << "z" << "\n";
    for(int i = 0; i < m_world->defectSiteIDs()->size(); i++)
    {
        int s = m_world->defectSiteIDs()->at(i);
        int x = m_world->electronGrid()->getIndexX(s);
        int y = m_world->electronGrid()->getIndexY(s);
        int z = m_world->electronGrid()->getIndexZ(s);
        stream << m_world->parameters()->currentStep << s << x << y << z << "\n";
    }
    stream.flush();
}

void Logger::saveElectronGridPotential()
{ 
    QFile file;
    QString name = m_outputdir.absoluteFilePath(QString("%1_%2_electron_grid.dat").arg(m_stub).arg(m_world->parameters()->currentStep));
    file.setFileName(name);
    QTextStream stream;
    stream.setDevice(&file);
    stream.setRealNumberPrecision(m_world->parameters()->outputPrecision);
    stream.setFieldWidth(m_world->parameters()->outputWidth);
    stream.setRealNumberNotation(QTextStream::ScientificNotation);
    stream.setFieldAlignment(QTextStream::AlignRight);
    if(!(file.open(QIODevice::WriteOnly | QIODevice::Text)) )
    {
        qFatal("can not open file %s", qPrintable(name));
    }
    stream << "step" << "s" << "x" << "y" << "z" << "v" << "\n";
    for(int i = 0; i < m_world->electronGrid()->xSize(); i++)
    {
        for(int j = 0; j < m_world->electronGrid()->ySize(); j++)
        {
            for(int k = 0; k < m_world->electronGrid()->zSize(); k++)
            {
                int s = m_world->electronGrid()->getIndexS(i, j, k);
                stream << m_world->parameters()->currentStep << s << i << j << k << m_world->electronGrid()->potential(s) << "\n";
            }
        }
    }
    stream.flush();
}

void Logger::saveHoleGridPotential()
{
    QFile file;
    QString name = m_outputdir.absoluteFilePath(QString("%1_%2_hole_grid.dat").arg(m_stub).arg(m_world->parameters()->currentStep));
    file.setFileName(name);
    QTextStream stream;
    stream.setDevice(&file);
    stream.setRealNumberPrecision(m_world->parameters()->outputPrecision);
    stream.setFieldWidth(m_world->parameters()->outputWidth);
    stream.setRealNumberNotation(QTextStream::ScientificNotation);
    stream.setFieldAlignment(QTextStream::AlignRight);
    if(!(file.open(QIODevice::WriteOnly | QIODevice::Text)) )
    {
        qFatal("can not open file %s", qPrintable(name));
    }
    stream << "step" << "s" << "x" << "y" << "z" << "v" << "\n";
    for(int i = 0; i < m_world->holeGrid()->xSize(); i++)
    {
        for(int j = 0; j < m_world->holeGrid()->ySize(); j++)
        {
            for(int k = 0; k < m_world->holeGrid()->zSize(); k++)
            {
                int s = m_world->holeGrid()->getIndexS(i, j, k);
                stream << m_world->parameters()->currentStep << s << i << j << k << m_world->holeGrid()->potential(s) << "\n";
            }
        }
    }
    stream.flush();
}

void Logger::saveCoulombEnergy()
{
    QFile out;
    QString name = m_outputdir.absoluteFilePath(QString("%1_%2_coulomb.dat").arg(m_stub).arg(m_world->parameters()->currentStep));
    out.setFileName(name);
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
                out.write(qPrintable(QString("%1 %2 %3 %4\n").arg(i, w).arg(j, w).arg(k, w).arg(   m_world->opencl()->getOutputHost(s), w, 'e', p)) );
            }
        }
    }
    out.close();
}

void Logger::saveTrapImage()
{
    //Create an empty image of the appropriate size
    QImage img(m_world->parameters()->gridX, m_world->parameters()->gridY, QImage::Format_ARGB32_Premultiplied);
    img.fill(0);
    //Create a painter to draw the image
    QPainter painter;
    painter.begin(&img);
    //Move the origin from top left to bottom right
    painter.scale(1.0, -1.0);
    painter.translate(0.0, -m_world->parameters()->gridY);
    //Resize the painter window to the grid dimensions
    painter.setWindow(QRect(0, 0, m_world->parameters()->gridX, m_world->parameters()->gridY));
    //Set the background to be white
    painter.fillRect(QRect(0, 0, m_world->parameters()->gridX, m_world->parameters()->gridY), Qt::white);
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
    img.save(m_outputdir.absoluteFilePath(QString("%1_%2_trap_image.png").arg(m_stub).arg(m_world->parameters()->currentStep)), "png", 100);
}

}
