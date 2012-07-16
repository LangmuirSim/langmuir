#include "keyvalueparser.h"
#include <QDebug>
#include <QRegExp>
#include <QStringList>
#include <QFile>
#include "output.h"
#include <ostream>

namespace Langmuir
{

KeyValueParser::KeyValueParser(World &world, QObject *parent) :
    QObject(parent), m_world(world)
{
    registerVariable("simulation.type", m_parameters.simulationType);
    registerVariable("random.seed", m_parameters.randomSeed);
    registerVariable("grid.z", m_parameters.gridZ);
    registerVariable("grid.y", m_parameters.gridY);
    registerVariable("grid.x", m_parameters.gridX);
    registerVariable("coulomb.carriers", m_parameters.coulombCarriers);
    registerVariable("defects.charge", m_parameters.defectsCharge);
    registerVariable("output.xyz", m_parameters.outputXyz);
    registerVariable("output.ids.on.delete", m_parameters.outputIdsOnDelete);
    registerVariable("output.coulomb", m_parameters.outputCoulomb);
    registerVariable("output.potential", m_parameters.outputPotential);
    registerVariable("output.is.on", m_parameters.outputIsOn);
    registerVariable("image.traps", m_parameters.imageTraps);
    registerVariable("image.defects", m_parameters.imageDefects);
    registerVariable("image.carriers", m_parameters.imageCarriers);
    registerVariable("iterations.print", m_parameters.iterationsPrint);
    registerVariable("iterations.real", m_parameters.iterationsReal);
    registerVariable("output.precision", m_parameters.outputPrecision);
    registerVariable("output.width", m_parameters.outputWidth);
    registerVariable("output.stub", m_parameters.outputStub);
    registerVariable("electron.percentage", m_parameters.electronPercentage);
    registerVariable("hole.percentage", m_parameters.holePercentage);
    registerVariable("seed.charges", m_parameters.seedCharges);
    registerVariable("defect.percentage", m_parameters.defectPercentage);
    registerVariable("trap.percentage", m_parameters.trapPercentage);
    registerVariable("trap.potential", m_parameters.trapPotential);
    registerVariable("gaussian.stdev", m_parameters.gaussianStdev);
    registerVariable("seed.percentage", m_parameters.seedPercentage);
    registerVariable("voltage.right", m_parameters.voltageRight);
    registerVariable("voltage.left", m_parameters.voltageLeft);
    registerVariable("temperature.kelvin", m_parameters.temperatureKelvin);
    registerVariable("source.rate", m_parameters.sourceRate);
    registerVariable("drain.rate", m_parameters.drainRate);
    registerVariable("use.opencl", m_parameters.useOpenCL);
    registerVariable("work.x", m_parameters.workX);
    registerVariable("work.y", m_parameters.workY);
    registerVariable("work.z", m_parameters.workZ);
    registerVariable("work.size", m_parameters.workSize);
    registerVariable("boltzmann.constant", m_parameters.boltzmannConstant, Variable::Constant);
    registerVariable("dielectric.constant", m_parameters.dielectricConstant, Variable::Constant);
    registerVariable("elementary.charge", m_parameters.elementaryCharge, Variable::Constant);
    registerVariable("permittivity.space", m_parameters.permittivitySpace, Variable::Constant);
    registerVariable("grid.factor", m_parameters.gridFactor, Variable::Constant);
    registerVariable("electrostatic.cutoff", m_parameters.electrostaticCutoff, Variable::Constant);
    registerVariable("electrostatic.prefactor", m_parameters.electrostaticPrefactor, Variable::Constant);
    registerVariable("inverse.kt", m_parameters.inverseKT, Variable::Constant);
    registerVariable("ok.cl", m_parameters.okCL, Variable::Constant);
    registerVariable("current.step", m_parameters.currentStep);
    registerVariable("simulation.start", m_parameters.simulationStart, Variable::Constant);
    registerVariable("hopping.range", m_parameters.hoppingRange);
    registerVariable("slope.z", m_parameters.slopeZ);
    registerVariable("source.metropolis", m_parameters.sourceMetropolis);
    registerVariable("source.coulomb", m_parameters.sourceCoulomb);
    registerVariable("recombination.rate", m_parameters.recombinationRate);
    registerVariable("output.ids.on.encounter", m_parameters.outputIdsOnEncounter);
    registerVariable("source.scale.area", m_parameters.sourceScaleArea);
}

KeyValueParser::~KeyValueParser()
{
}

void KeyValueParser::parse(const QString &line)
{
    // Remove whitespace
    QString parsed = line.trimmed();

    // Remove comments
    parsed = parsed.replace(QRegExp("\\s*#.*$"),"");

    // Ignore empty lines, but not an error
    if (parsed.size() == 0)
    {
        return;
    }

    // Make sure there is only one '=' sign, if not, it is an error
    if (parsed.count('=') != 1)
    {
        qFatal("parse error: missing equal sign\n\tline: %s",
               qPrintable(line.trimmed()));
    }

    // Split into tokens
    QStringList tokens = parsed.split('=', QString::KeepEmptyParts);

    // Make sure there are two parts
    if (tokens.size() != 2)
    {
        qFatal("parse error: line does not split into two at equal sign\n\tline: %s",
               qPrintable(line.trimmed()));
    }

    // Get the key, make sure it is ok
    QString key = tokens.at(0).trimmed().toLower();
    if (key.isEmpty())
    {
        qFatal("parse error: key is empty\n\tline: %s",
               qPrintable(line.trimmed()));
    }

    // Find the variable in the map
    QMap<QString, Variable*>::iterator it = m_variableMap.find(key);

    // Make sure we got a valid key
    if (it == m_variableMap.end())
    {
        qFatal("parse error: invalid key\n\tline: %s",
               qPrintable(line.trimmed()));
    }

    // Get the value
    QString value = tokens.at(1).trimmed()
            .replace(QRegExp("^\"+"),"").replace(QRegExp("\"+$"),"");
    if (value.isEmpty())
    {
        qWarning("warning! value is empty for key: %s",
               qPrintable(key.trimmed()));
        return;
    }

    // Convert and store the value
    it.value()->read(value);
}

void KeyValueParser::save(const QString& fileName)
{
    if (!m_world.parameters().outputIsOn) return;
    OutputInfo info(fileName, &m_world.parameters());
    QFile handle(info.absoluteFilePath());
    if (!handle.open(QIODevice::WriteOnly|QIODevice::Text))
    {
        qFatal("can not open file: %s", qPrintable(info.absoluteFilePath()));
    }
    QTextStream stream(&handle);
    stream << "[Parameters]";
    foreach (Variable *variable, m_variableMap)
    {
        if (!variable->isConstant())
        {
            stream << '\n' << *variable;
        }
    }
    stream.flush();
    handle.close();
}

std::ostream& operator<<(std::ostream& stream, const KeyValueParser &keyValueParser)
{
    foreach (Variable *variable, keyValueParser.m_variableMap)
    {
        if (!variable->isConstant())
        {
            stream << '\n' << *variable;
        }
    }
    return stream;
}

SimulationParameters& KeyValueParser::parameters()
{
    checkSimulationParameters(m_parameters);
    return m_parameters;
}

}
