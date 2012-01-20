#include "openclhelper.h"
#include "inputparser.h"
#include "chargeagent.h"
#include "cubicgrid.h"
#include "logger.h"
#include "world.h"

namespace Langmuir
{

DataFile::DataFile(QString name, int width, int percision, QObject *parent)
    : QObject(parent)
{
    m_file.setFileName(name);
    m_stream.setDevice(&m_file);
    Q_ASSERT(m_file.open(QIODevice::WriteOnly|QIODevice::Text));
    m_stream << qSetRealNumberPrecision(percision)
             << qSetFieldWidth(width)
             << scientific
             << right;
}

void DataFile::report()
{
    for ( int i = 0; i < m_columns.size(); i++ )
    {
        m_columns[i]->report(m_stream);
    }
}









/*
Logger::Logger(World * world, QObject *parent): QObject(parent), m_world(world)
{
    m_outputdir = QDir(m_world->parameters()->outputPath);
    if(!m_outputdir.exists()) { qFatal("%s does not exist", qPrintable(m_outputdir.path()));}    

    if ( m_world->parameters()->outputIdsOnDelete )
    {
        createFileStream(m_carrierStream,m_carrierFile,generateFileName("onDelete"));
        m_carrierStream << "step";
        ChargeAgent::setTitles(m_carrierStream,*m_world->parameters()); m_carrierStream << "\n";
    }

    createFileStream(m_stepStream,m_stepFile,generateFileName(""));

    QString string;
    QTextStream stream(&string);
    stream << this->metaObject()->className() << "(" << this << ")";
    setObjectName(string);
}

void Logger::flush()
{

}

void Logger::report( ChargeAgent &charge )
{
    m_carrierStream << m_world->parameters()->currentStep << charge << "\n";
}

void report( FluxAgent &flux )
{

}

Logger::~Logger()
{
    flush();
}

void Logger::saveElectronIDs()
{
    if(m_world->parameters()->electronPercentage<=0)return;

    QFile file; QTextStream stream;
    createFileStream(stream,file,generateFileName("electronIDs"));

    stream << "step";
    ChargeAgent::setTitles(stream,*m_world->parameters()); stream << "\n";
    for(int i = 0; i < m_world->electrons()->size(); i++)
    {
        stream << m_world->parameters()->currentStep << *m_world->electrons()->at(i) << "\n";
    }
}

void Logger::saveHoleIDs()
{
    if(m_world->parameters()->holePercentage<=0)return;

    QFile file; QTextStream stream;
    createFileStream(stream,file,generateFileName("holeIDs"));

    stream << "step";
    ChargeAgent::setTitles(stream,*m_world->parameters()); stream << "\n";
    for(int i = 0; i < m_world->holes()->size(); i++)
    {
        stream << m_world->parameters()->currentStep << *m_world->holes()->at(i) << "\n";
    }
}

void Logger::saveTrapIDs()
{
    if(m_world->trapSiteIDs()->size()==0)return;

    QFile file; QTextStream stream;
    createFileStream(stream,file,generateFileName("trapIDs"));

    stream << "si" << "xi" << "yi" << "zi" << "\n";
    for(int i = 0; i < m_world->trapSiteIDs()->size(); i++)
    {
        int s = m_world->trapSiteIDs()->at(i);
        stream << s
               << m_world->electronGrid()->getIndexX(s)
               << m_world->electronGrid()->getIndexY(s)
               << m_world->electronGrid()->getIndexZ(s)
               << "\n";
    }
    stream.flush();
}

void Logger::saveDefectIDs()
{
    if(m_world->defectSiteIDs()->size()==0)return;

    QFile file; QTextStream stream;
    createFileStream(stream,file,generateFileName("defectIDs"));

    stream << "si" << "xi" << "yi" << "zi" << "\n";
    for(int i = 0; i < m_world->defectSiteIDs()->size(); i++)
    {
        int s = m_world->defectSiteIDs()->at(i);
        stream << s
               << m_world->electronGrid()->getIndexX(s)
               << m_world->electronGrid()->getIndexY(s)
               << m_world->electronGrid()->getIndexZ(s)
               << "\n";
    }
    stream.flush();
}

void Logger::saveElectronGridPotential()
{ 
    QFile file; QTextStream stream;
    createFileStream(stream,file,generateFileName("eGrid"));

    stream << "si" << "xi" << "yi" << "zi" << "vi" << "\n";
    for(int i = 0; i < m_world->electronGrid()->xSize(); i++)
    {
        for(int j = 0; j < m_world->electronGrid()->ySize(); j++)
        {
            for(int k = 0; k < m_world->electronGrid()->zSize(); k++)
            {
                int s = m_world->electronGrid()->getIndexS(i, j, k);
                stream << s
                       << i
                       << j
                       << k
                       << m_world->electronGrid()->potential(s)
                       << "\n";
            }
        }
    }
    stream.flush();
}

void Logger::saveHoleGridPotential()
{
    QFile file; QTextStream stream;
    createFileStream(stream,file,generateFileName("hGrid"));

    stream << "si" << "xi" << "yi" << "zi" << "vi" << "\n";
    for(int i = 0; i < m_world->holeGrid()->xSize(); i++)
    {
        for(int j = 0; j < m_world->holeGrid()->ySize(); j++)
        {
            for(int k = 0; k < m_world->holeGrid()->zSize(); k++)
            {
                int s = m_world->holeGrid()->getIndexS(i, j, k);
                stream << s
                       << i
                       << j
                       << k
                       << m_world->holeGrid()->potential(s)
                       << "\n";
            }
        }
    }
    stream.flush();
}

void Logger::saveCoulombEnergy()
{
    QFile file; QTextStream stream;
    createFileStream(stream,file,generateFileName("coulomb"));

    stream << "si" << "xi" << "yi" << "zi" << "vi" << "\n";
    for(int i = 0; i < m_world->electronGrid()->xSize(); i++)
    {
        for(int j = 0; j < m_world->electronGrid()->ySize(); j++)
        {
            for(int k = 0; k < m_world->electronGrid()->zSize(); k++)
            {
                int s = m_world->electronGrid()->getIndexS(i, j, k);
                stream << s
                       << i
                       << j
                       << k
                       << m_world->opencl()->getOutputHost(s)
                       << "\n";
            }
        }
    }
    stream.flush();
}

void Logger::saveTrapImage()
{
    if(m_world->trapSiteIDs()->size()==0)return;
    QImage image;
    QPainter painter;
    beginImage(image,painter,Qt::white);
    drawSites(painter, *m_world->trapSiteIDs(), Qt::blue, 0 );
    endImage(image,painter,"traps",5);
}

void Logger::saveDefectImage()
{
    if(m_world->defectSiteIDs()->size()==0)return;
    QImage image;
    QPainter painter;
    beginImage(image,painter,Qt::white);
    drawSites(painter, *m_world->trapSiteIDs(), Qt::red, 0 );
    endImage(image,painter,"defects",5);
}

void Logger::saveElectronImage()
{
    if(m_world->parameters()->electronPercentage<=0)return;
    QImage image;
    QPainter painter;
    beginImage(image,painter,Qt::white);
    drawCharges(painter, *m_world->electrons(), Qt::green, 0 );
    endImage(image,painter,"electrons",5);
}

void Logger::saveHoleImage()
{
    if(m_world->parameters()->holePercentage<=0)return;
    QImage image;
    QPainter painter;
    beginImage(image,painter,Qt::white);
    drawCharges(painter, *m_world->holes(), Qt::cyan, 0 );
    endImage(image,painter,"holes",5);
}

void Logger::saveImage()
{
    QImage image;
    QPainter painter;
    beginImage(image,painter,Qt::white);
    if (m_world->trapSiteIDs()->size()>0) { drawSites(painter, *m_world->trapSiteIDs(), Qt::blue, 0 ); }
    if (m_world->defectSiteIDs()->size()>0) { drawSites(painter, *m_world->trapSiteIDs(), Qt::red, 0 ); }
    if (m_world->parameters()->electronPercentage>0) { drawCharges(painter, *m_world->electrons(), Qt::green, 0 ); }
    if (m_world->parameters()->holePercentage>0) { drawCharges(painter, *m_world->holes(), Qt::cyan, 0 ); }
    endImage(image,painter,"all",5);
}

void Logger::beginImage( QImage &image, QPainter &painter, QColor background )
{
    image = QImage(m_world->parameters()->gridX, m_world->parameters()->gridY, QImage::Format_ARGB32_Premultiplied);
    image.fill(0);
    painter.begin(&image);
    painter.scale(1.0, -1.0);
    painter.translate(0.0, -m_world->parameters()->gridY);
    painter.setWindow(QRect(0, 0, m_world->parameters()->gridX, m_world->parameters()->gridY));
    painter.fillRect(QRect(0, 0, m_world->parameters()->gridX, m_world->parameters()->gridY), background);
}

void Logger::endImage( QImage &image, QPainter &painter, QString name, int scale )
{
    painter.end();
    image = image.scaled(scale*image.width(), scale*image.height(), Qt::KeepAspectRatioByExpanding);
    image.save(generateFileName(name,"png"), "png", 100);
}

void Logger::drawSites( QPainter &painter, QList<int> &sites, QColor color, int layer )
{
    if (sites.size()<=0)return;
    painter.setPen(color);
    for(int i = 0; i < sites.size(); i++)
    {
        int ndx = sites[i];
        if(m_world->electronGrid()->getIndexZ(ndx)== layer)
        {
            painter.drawPoint(QPoint(m_world->electronGrid()->getIndexX(ndx), m_world->electronGrid()->getIndexY(ndx)));
        }
    }
}

void Logger::drawCharges( QPainter &painter, QList<ChargeAgent *> &charges,QColor color, int layer )
{
    if (charges.size()<=0)return;
    painter.setPen(color);
    for(int i = 0; i < charges.size(); i++)
    {
        int ndx = charges[i]->site();
        if(m_world->electronGrid()->getIndexZ(ndx)== layer)
        {
            painter.drawPoint(QPoint(m_world->electronGrid()->getIndexX(ndx), m_world->electronGrid()->getIndexY(ndx)));
        }
    }
}

void Logger::setStreamParameters( QTextStream &stream )
{
    stream.setRealNumberPrecision(m_world->parameters()->outputPrecision);
    stream.setFieldWidth(m_world->parameters()->outputWidth);
    stream.setRealNumberNotation(QTextStream::ScientificNotation);
    stream.setFieldAlignment(QTextStream::AlignRight);
}

void Logger::createFileStream( QTextStream &stream, QFile &file, QString fileName )
{
    file.setFileName(fileName);
    stream.setDevice(&file);
    setStreamParameters(stream);
    if(!(file.open(QIODevice::WriteOnly|QIODevice::Text)))
    {
        qFatal("can not open file %s", qPrintable(fileName));
    }
}

QString Logger::generateFileName( QString identifier, QString extension )
{
     int w1 = int(log10(m_world->parameters()->simulationSteps))+1;
     int w2 = int(log10(m_world->parameters()->iterationsReal))+1;

     if ( ! identifier.isEmpty() ) { identifier = "-" + identifier; }

     QString name = m_outputdir.absoluteFilePath(
                 QString("%1-%2-%3%4.%5")
                 .arg(m_world->parameters()->outputStub)
                 .arg(m_world->parameters()->currentSimulation,w1,10,QLatin1Char('0'))
                 .arg(m_world->parameters()->currentStep,w2,10,QLatin1Char('0'))
                 .arg(identifier)
                 .arg(extension)
                 );
     return name;
}

void Logger::saveParameters()
{

}
*/
}
