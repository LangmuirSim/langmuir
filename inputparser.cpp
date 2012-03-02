#include "inputparser.h"
#include "output.h"
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QRegExp>
#include <QStringList>

namespace Langmuir
{

InputParser::InputParser(QObject *parent) :
    QObject(parent)
{
    createSteps(0);
}

void InputParser::setMap(int step)
{
    createVariable("simulation.type"        , m_parameters[step].simulationType      , step);
    createVariable("random.seed"            , m_parameters[step].randomSeed          , step);

    createVariable("grid.z"                 , m_parameters[step].gridZ               , step);
    createVariable("grid.y"                 , m_parameters[step].gridY               , step);
    createVariable("grid.x"                 , m_parameters[step].gridX               , step);

    createVariable("coulomb.carriers"       , m_parameters[step].coulombCarriers     , step);
    createVariable("coulomb.defects"        , m_parameters[step].coulombDefects      , step);
    createVariable("defects.charge"         , m_parameters[step].defectsCharge       , step);

    createVariable("current.simulation"     , m_parameters[step].currentSimulation   , step);
    createVariable("output.is.on"           , m_parameters[step].outputIsOn          , step);
    createVariable("output.overwrite"       , m_parameters[step].outputOverwrite     , step);
    createVariable("output.stub"            , m_parameters[step].outputStub          , step);
    createVariable("output.ids.on.delete"   , m_parameters[step].outputIdsOnDelete   , step);
    createVariable("output.coulomb"         , m_parameters[step].outputCoulomb       , step);
    createVariable("output.potential"       , m_parameters[step].outputPotential     , step);
    createVariable("output.image"           , m_parameters[step].outputImage         , step);
    createVariable("output.xyz"             , m_parameters[step].outputXyz           , step);
    createVariable("output.precision"       , m_parameters[step].outputPrecision     , step);
    createVariable("output.width"           , m_parameters[step].outputWidth         , step);
    createVariable("output.path"            , m_parameters[step].outputPath          , step);

    createVariable("iterations.print"       , m_parameters[step].iterationsPrint     , step);
    createVariable("iterations.real"        , m_parameters[step].iterationsReal      , step);
    createVariable("iterations.warmup"      , m_parameters[step].iterationsWarmup    , step);

    createVariable("electron.percentage"    , m_parameters[step].electronPercentage  , step);
    createVariable("hole.percentage"        , m_parameters[step].holePercentage      , step);
    createVariable("seed.charges"           , m_parameters[step].seedCharges         , step);
    createVariable("defect.percentage"      , m_parameters[step].defectPercentage    , step);
    createVariable("trap.percentage"        , m_parameters[step].trapPercentage      , step);
    createVariable("trap.potential"         , m_parameters[step].trapPotential       , step);
    createVariable("gaussian.average"       , m_parameters[step].gaussianAverg       , step);
    createVariable("gaussian.stdev"         , m_parameters[step].gaussianStdev       , step);
    createVariable("seed.percentage"        , m_parameters[step].seedPercentage      , step);

    createVariable("voltage.right"          , m_parameters[step].voltageRight        , step);
    createVariable("voltage.left"           , m_parameters[step].voltageLeft       , step);
    createVariable("temperature.kelvin"     , m_parameters[step].temperatureKelvin   , step);

    createVariable("source.rate"            , m_parameters[step].sourceRate          , step);
    createVariable("drain.rate"             , m_parameters[step].drainRate           , step);

    createVariable("use.opencl"             , m_parameters[step].useOpenCL           , step);
    createVariable("work.x"                 , m_parameters[step].workX               , step);
    createVariable("work.y"                 , m_parameters[step].workY               , step);
    createVariable("work.z"                 , m_parameters[step].workZ               , step);
    createVariable("work.size"              , m_parameters[step].workSize            , step);

    createVariable("configuration.file"     , m_parameters[step].configurationFile   , step);

    createVariable("boltzmann.constant"     , m_parameters[step].boltzmannConstant     , step, false);
    createVariable("dielectric.constant"    , m_parameters[step].dielectricConstant    , step, false);
    createVariable("elementary.charge"      , m_parameters[step].elementaryCharge      , step, false);
    createVariable("permittivity.space"     , m_parameters[step].permittivitySpace     , step, false);
    createVariable("grid.factor"            , m_parameters[step].gridFactor            , step, false);
    createVariable("electrostatic.cutoff"   , m_parameters[step].electrostaticCutoff   , step, false);
    createVariable("electrostatic.prefactor", m_parameters[step].electrostaticPrefactor, step, false);
    createVariable("inverse.KT"             , m_parameters[step].inverseKT             , step, false);
    createVariable("ok.CL"                  , m_parameters[step].okCL                  , step, false);
    createVariable("current.step"           , m_parameters[step].currentStep           , step, false);
    createVariable("iterations.total"       , m_parameters[step].iterationsTotal       , step, false);
}

InputParser::~InputParser()
{
    for (int i = 0; i < m_variables.size(); i++)
    {
        foreach ( AbstractVariable *var, m_variables[i] )
        {
            delete var;
        }
        m_variables[i].clear();
    }
}

SimulationParameters& InputParser::getParameters(int step)
{
    checkStep(step);
    m_parameters[step].check();
    return m_parameters[step];
}

void InputParser::saveParametersXML(int step)
{
    checkStep(step);
}

void InputParser::saveParameters(int step)
{
    if ( ! m_parameters[step].outputIsOn ) return;
    checkStep(step);
    OutputStream stream("%path/%stub-%sim.inp",&m_parameters[step],0,this);
    foreach(AbstractVariable *var, m_variables[step])
    {
        stream << QString("%1 = %2\n").arg(var->key(),-25).arg(var->value(15),0);
    }
    stream << flush;
}

void InputParser::checkStep(int step)
{
    if (step < 0 || step >= m_parameters.size() || step >= m_variables.size())
    {
        qFatal("Parser requested step(%d) >= steps(%d) defined",steps(),step);
    }
}

void InputParser::setValue(QString key, QString value, int step)
{
    createSteps(step);
    if (!(m_variables[step].contains(key)))
    {
        qFatal("Parser::setValue requested key(%s) which is not a valid key",qPrintable(key));
    }
    m_variables[step][key]->setValue(value);
}

int InputParser::steps()
{
    return m_parameters.size();
}

void InputParser::createSteps(int toStep)
{
    for (int i = steps()-1; i < toStep; i++)
    {
        m_parameters.push_back(SimulationParameters());
        m_variables.push_back(QMap<QString,AbstractVariable*>());
        setMap(steps()-1);
    }
}

void InputParser::AbstractVariable::convert( QString string, QString &result )
{
    result = string;
}

void InputParser::AbstractVariable::convert( QString string, double &result )
{
    bool ok = false;
    double converted = string.toDouble(&ok);
    if ( ok )
    {
        result = converted;
    }
    else
    {
        qFatal("Parser::AbstractVariable can not convert %s to double",qPrintable(string));
    }
}

void InputParser::AbstractVariable::convert( QString string, bool &result )
{
    if ( string.trimmed().toLower() == "false" )
    {
        result = false;
        return;
    }
    if ( string.trimmed().toLower() == "true" )
    {
        result = true;
        return;
    }
    int converted = 0;
    convert(string,converted);
    if ( converted <= 0 )
    {
        result = false;
    }
    else
    {
        result = true;
    }
}

void InputParser::AbstractVariable::convert( QString string, int &result )
{
    bool ok = false;
    int converted = string.toInt(&ok);
    if ( ok )
    {
        result = converted;
    }
    else
    {
        qFatal("Parser::AbstractVariable can not convert %s to int",qPrintable(string));
    }
}

void InputParser::AbstractVariable::convert( QString string, unsigned int &result )
{
    bool ok = false;
    int converted = string.toUInt(&ok);
    if ( ok )
    {
        result = converted;
    }
    else
    {
        qFatal("Parser::AbstractVariable can not convert %s to unsigned int",qPrintable(string));
    }
}

void InputParser::parserXML(QString fileName)
{
}

void InputParser::parseKeyValue(QString fileName)
{
    QFile     handle(fileName);
    if(! handle.open(QIODevice::ReadOnly | QIODevice::Text))
        qFatal("can not open input file: %s", qPrintable(fileName));

    QRegExp regex1("\\s*=\\s*");
    QRegExp regex2("\\s*,\\s*");
    QRegExp regex3("\\s*\\[\\s*");
    QRegExp regex4("\\s*\\]\\s*");
    QRegExp regex5("#.*$");

    int lineNumber = 0;
    while(!handle.atEnd())
    {
        QString info;
        info += QString(" file(%1)").arg(fileName);
        info += QString(" line(%1)").arg(lineNumber);

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
            if(line.count("=") != 1)
            {
                qFatal("Parser::parseKeyValue line without '=' sign;%s",qPrintable(info));
            }

            QStringList tokens = line.split("=", QString::SkipEmptyParts);
            if(tokens.size() != 2)
            {
                qFatal("Parser::parseKeyValue line with '=' sign doesn't split into 2 tokens;%s",qPrintable(info));
            }

            QString key = tokens[0].trimmed().toLower();

            if(key.isEmpty())
            {
                qFatal("Parser::parseKeyValue key is empty;%s",qPrintable(info));
            }
            info += QString(" key(%1)").arg(key);

            QStringList values = tokens[1].split(",", QString::SkipEmptyParts);
            if(values.size()== 0)
            {
                qFatal("Parser::parseKeyValue values are empty;%s",qPrintable(info));
            }

            for(int i = 0; i < values.size(); i++)
            {
                QString value = values[i].trimmed();
                if(value.isEmpty())
                {
                    qFatal("Parser::parseKeyValue value is empty;%s",qPrintable(info));
                }
                setValue(key,value,i);
            }
        }
        lineNumber += 1;
    }
    // propagate unspecified values forward
    if (m_parameters.size() > 1)
    {
        for (int i = 1; i < m_parameters.size(); i++)
        {
            foreach(AbstractVariable *var, m_variables[i])
            {
                if (!var->wasSet())
                {
                    AbstractVariable *last = m_variables[i-1][var->key()];
                    if ( var->isWritable() ) var->setValue(last->value(100));
                }
            }
        }
    }
    // check the parameters at each step
    for (int i = 0; i < m_parameters.size(); i++)
    {
        m_parameters[i].currentSimulation = m_parameters[0].currentSimulation + i;
        //m_parameters[i].simulationSteps = steps();
        m_parameters[i].check();
    }
}

}
