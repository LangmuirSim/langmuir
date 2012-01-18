#include "inputparser.h"
#include "simulation.h"
#include <QtCore>

namespace Langmuir
{

SimulationParameters::SimulationParameters()
{
    randomSeed = -1;

    gridZ = 1;
    gridY = 128;
    gridX = 128;

    coulombCarriers = false;
    coulombDefects = false;
    defectsCharge = -1;

    outputElectronIDs = false;
    outputHoleIDs = false;
    outputDefectIDs = false;
    outputTrapIDs = false;

    outputCoulomb = false;
    outputElectronGrid = false;
    outputHoleGrid = false;
    outputTrapImage = false;
    outputOnDelete = false;

    iterationsPrint = 10;
    iterationsReal = 1000;
    iterationsWarmup = 1000;
    outputPrecision = 5;
    outputWidth = 20;
    outputPath = QDir::currentPath();

    electronPercentage = 0.01;
    holePercentage = 0.00;
    seedCharges = false;
    defectPercentage = 0.00;
    trapPercentage = 0.00;
    trapPotential = 0.10;
    gaussianAverg = 0.00;
    gaussianStdev = 0.00;
    seedPercentage = 1.0;

    voltageDrain = 0.00;
    voltageSource = 0.00;
    temperatureKelvin = 300.0;

    useOpenCL = false;
    workX = 4;
    workY = 4;
    workZ = 4;
    workSize = 256;
    kernelsPath = QDir::currentPath();

    boltzmannConstant = 1.3806504e-23;
    dielectricConstant = 3.5;
    elementaryCharge = 1.60217646e-19;
    permittivitySpace = 8.854187817e-12;
    gridFactor = 1e-9;
    electrostaticCutoff = 50;
    electrostaticPrefactor = elementaryCharge /
     (4.0 * M_PI * dielectricConstant * permittivitySpace * gridFactor);
    inverseKT = 1.0 /(boltzmannConstant * temperatureKelvin);
    okCL = false;
    currentStep = 0;
    currentSimulation = 0;
    outputStub = "out";
    iterationsTotal = iterationsReal + iterationsWarmup;

    check();
}

SimulationParameters& InputParserTemp::getParameters(int step)
{
    parameter("random.seed",m_parameters.randomSeed,step);

    parameter("grid.z",m_parameters.gridZ,step);
    parameter("grid.y",m_parameters.gridY,step);
    parameter("grid.x",m_parameters.gridX,step);

    parameter("coulomb.carriers",m_parameters.coulombCarriers,step);
    parameter("coulomb.defects",m_parameters.coulombDefects,step);
    parameter("defects.charge",m_parameters.defectsCharge,step);

    parameter("output.electron.ids",m_parameters.outputElectronIDs,step);
    parameter("output.hole.ids",m_parameters.outputHoleIDs,step);
    parameter("output.defect.ids",m_parameters.outputDefectIDs,step);
    parameter("output.trap.ids",m_parameters.outputTrapIDs,step);

    parameter("output.coulomb",m_parameters.outputCoulomb,step);
    parameter("output.electron.grid",m_parameters.outputElectronGrid,step);
    parameter("output.hole.grid",m_parameters.outputHoleGrid,step);
    parameter("output.trap.image",m_parameters.outputTrapImage,step);
    parameter("output.on.delete",m_parameters.outputOnDelete,step);

    parameter("iterations.print",m_parameters.iterationsPrint,step);
    parameter("iterations.real",m_parameters.iterationsReal,step);
    parameter("iterations.warmup",m_parameters.iterationsWarmup,step);
    parameter("output.percision",m_parameters.outputPrecision,step);
    parameter("output.width",m_parameters.outputWidth,step);
    parameter("output.path",m_parameters.outputPath,step);

    parameter("electron.percentage",m_parameters.electronPercentage ,step);
    parameter("hole.percentage",m_parameters.holePercentage,step);
    parameter("seed.charges",m_parameters.seedCharges,step);
    parameter("defect.percentage",m_parameters.defectPercentage,step);
    parameter("trap.percentage",m_parameters.trapPercentage,step);
    parameter("trap.potential",m_parameters.trapPotential,step);
    parameter("gaussian.average",m_parameters.gaussianAverg,step);
    parameter("gaussian.stdev",m_parameters.gaussianStdev,step);
    parameter("seed.percentage",m_parameters.seedPercentage,step);

    parameter("voltage.drain",m_parameters.voltageDrain,step);
    parameter("voltage.source",m_parameters.voltageSource,step);
    parameter("temperature.kelvin",m_parameters.temperatureKelvin,step);

    parameter("use.opencl",m_parameters.useOpenCL,step);
    parameter("work.x",m_parameters.workX,step);
    parameter("work.y",m_parameters.workY,step);
    parameter("work.z",m_parameters.workZ,step);
    parameter("work.size",m_parameters.workSize,step);
    parameter("kernels.path",m_parameters.kernelsPath,step);

    m_parameters.currentStep = 0;
    m_parameters.currentSimulation = step;

    m_parameters.check();

    return m_parameters;
}

void SimulationParameters::check()
{
    electrostaticPrefactor = elementaryCharge /
     (4.0 * M_PI * dielectricConstant * permittivitySpace * gridFactor);
    inverseKT = 1.0 /(boltzmannConstant * temperatureKelvin);

    Q_ASSERT( defectPercentage <= 1.00 - trapPercentage );

    Q_ASSERT( gridZ >= 1 );
    Q_ASSERT( gridY >= 1 );
    Q_ASSERT( gridX >= 1 );

    Q_ASSERT( iterationsPrint >= 0 );
    Q_ASSERT( iterationsReal >= 0 );
    Q_ASSERT( iterationsWarmup >= 0 );

    if(outputWidth <(outputPrecision + 8)) { outputWidth = outputPrecision + 8; }
    Q_ASSERT( outputWidth >= 8 );

    Q_ASSERT( electronPercentage >= 0.00 && electronPercentage <= 1.00 );
    Q_ASSERT( holePercentage >= 0.00 && holePercentage <= 1.00 );
    Q_ASSERT( defectPercentage >= 0.00 && defectPercentage <= 1.00 );
    Q_ASSERT( trapPercentage >= 0.00 && trapPercentage <= 1.00 );
    Q_ASSERT( seedPercentage >= 0.00 && seedPercentage <= 1.00 );

    Q_ASSERT( gaussianStdev >= 0.00 );
    Q_ASSERT( temperatureKelvin >= 0.00 );

    if ( outputElectronIDs ) { Q_ASSERT( electronPercentage > 0 ); }
    if ( outputHoleIDs ) { Q_ASSERT( holePercentage > 0 ); }

    Q_ASSERT( iterationsReal % iterationsPrint == 0 );
    Q_ASSERT( iterationsWarmup % iterationsPrint == 0 );
}

InputParserTemp::InputParserTemp(const QString &fileName, QObject *parent ) : QObject(parent)
{
    m_parameters.outputStub = fileName.split(".",QString::SkipEmptyParts)[0];

    getParameters(-1);
    parseFile(fileName);
    getParameters(-1);
}

void InputParserTemp::parseFile(const QString &fileName)
{
    QFile     handle(fileName);
    QFileInfo   info(fileName);

    if(! handle.open(QIODevice::ReadOnly | QIODevice::Text))
        qFatal("can not open input file: %s", qPrintable(fileName));

    QRegExp regex1("\\s*=\\s*");
    QRegExp regex2("\\s*, \\s*");
    QRegExp regex3("\\s*\\[\\s*");
    QRegExp regex4("\\s*\\]\\s*");
    QRegExp regex5("#.*$");

    int lineNumber = 0;
    while(!handle.atEnd())
    {
        QString unaltered = handle.readLine().trimmed();
        QString line = unaltered;
        line = line.replace(regex1, "="  );
        line = line.replace(regex2, ", " );
        line = line.replace(regex3, ""   );
        line = line.replace(regex4, ""   );
        line = line.replace(regex5, ""   );
        line = line.trimmed();

        if(line.length()> 0)
        {
            if(line.count("=")!= 1)
            {
                qFatal("InputParser syntax error:\n"
                       " file: %s\n"
                       " line: %d\n"
                       " text: %s\n"
                       " what: line must contain 1 equal sign.", 
                       qPrintable(info.fileName()), lineNumber, qPrintable(unaltered));
            }

            QStringList tokens = line.split("=", QString::SkipEmptyParts);

            if(tokens.size()!= 2)
            {
                qFatal("InputParser syntax error:\n"
                       " file: %s\n"
                       " line: %d\n"
                       " text: %s\n"
                       " what: line does not split into 2 sections at the equal sign.", 
                       qPrintable(info.fileName()), lineNumber, qPrintable(unaltered));
            }

            QString key = tokens[0].trimmed().toLower();

            if(key.isEmpty())
            {
                qFatal("InputParser syntax error:\n"
                       " file: %s\n"
                       " line: %d\n"
                       " text: %s\n"
                       " what: key is an empty string", 
                       qPrintable(info.fileName()), lineNumber, qPrintable(unaltered));
            }

            QStringList values = tokens[1].split(",", QString::SkipEmptyParts);

            if(values.size()== 0)
            {
                qFatal("InputParser syntax error:\n"
                       " file: %s\n"
                       " line: %d\n"
                       " text: %s\n"
                       "  key: %s\n"
                       " what: key specifies no values", 
                       qPrintable(info.fileName()), lineNumber, qPrintable(unaltered), qPrintable(key));
            }

            for(int i = 0; i < values.size(); i++)
            {
                QString value = values[i].trimmed();

                if(value.isEmpty())
                {
                    qFatal("InputParser syntax error:\n"
                           " file: %s\n"
                           " line: %d\n"
                           " text: %s\n"
                           "  key: %s\n"
                           " what: key value %d is an empty string", 
                           qPrintable(info.fileName()), lineNumber, qPrintable(unaltered), qPrintable(key), i);
                }

                if(! m_map.contains(key))
                {
                    qFatal("invalid key found: %s",qPrintable(key));
                }
                m_map[key].push_back(value);
            }
        }
        lineNumber += 1;
    }

    m_steps = 1;
    for(QMap<QString, QVector<QVariant> >::iterator i = m_map.begin(); i != m_map.end(); i++)
    {
        QVector<QVariant> & valueList = i.value();

        if (  i.value().size() > 1 )
        {
            valueList.pop_front();
        }
        for ( int j = 0; j < valueList.size(); j++ )
        {
            if ( valueList.count( valueList[j] ) != 1 )
            {
                qFatal("duplicate value found for key: %s",qPrintable(i.key()));
            }
        }
        if ( valueList.size() > m_steps )
        {
            m_steps = valueList.size();
        }
    }
}

QString InputParserTemp::mapToQString()
{
    QString result = "";
    for(QMap<QString, QVector<QVariant> >::iterator i = m_map.begin(); i != m_map.end(); i++)
    {
        QVector< QVariant > & valueList = i.value();

        QString valuesString;
        for(int j = 0; j < valueList.size(); j++)
        {
            switch( i.value().at(j).type() )
            {
            case QVariant::String:
            {
                valuesString += QString("%1,").arg( valueList[j].value<QString>() );
                break;
            }
            case QVariant::Double:
            {
                valuesString += QString("%1,").arg( valueList[j].value<double>() );
                break;
            }
            case QVariant::Bool:
            {
                valuesString += QString("%1,").arg( valueList[j].value<bool>() );
                break;
            }
            case QVariant::Int:
            {
                valuesString += QString("%1,").arg( valueList[j].value<int>() );
                break;
            }
            default:
            {
                valuesString += "???";
            }
            }
        }
        valuesString = valuesString.remove(valuesString.size()-1,1);

        if ( valueList.size() == 1 )
        {
            result += QString("%1 %2 %3\n").arg(i.key(),-30).arg("=").arg(valuesString);
        }

        else
        {
            result += QString("%1 %2 [%3]\n").arg(i.key(),-30).arg("=").arg(valuesString);
        }
    }
    result = result.remove(result.size()-1,1);
    return result;
}

int InputParserTemp::steps()
{
    return m_steps;
}

}
