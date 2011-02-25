#include "inputparser.h"
#include "simulation.h"

#include <QtCore/QFile>
#include <QtCore/QIODevice>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QDebug>

using namespace std;

namespace Langmuir
{

  InputParser::InputParser (const QString & fileName)
  {
    if (s_variables.isEmpty ())
      initializeVariables ();

    // Open up this input file
    QFile *file = new QFile (fileName);
    if (!file->open (QIODevice::ReadOnly | QIODevice::Text))
      {
        qDebug ("error opening input file: %s", qPrintable (fileName));
        qFatal ("bad input");
      }

    // Now read the input file in, line by line
    while (!file->atEnd ())
        processLine (file);

    //fix errors parameter conflicts
    if (m_parameters.outputWidth < (m_parameters.outputPrecision + 15))
      {
        m_parameters.outputWidth = m_parameters.outputPrecision + 15;
      }
    if (m_parameters.outputWidth < 20)
      {
        m_parameters.outputWidth = 20;
      }
    if ( m_parameters.defectPercentage > 1.00 - m_parameters.trapPercentage ) qFatal("percent defects + percent traps > 100 percent");

  }

  bool InputParser::simulationParameters (SimulationParameters * par,
                                          int step)
  {
    if (step < 0 || step >= m_parameters.variableSteps)        // Invalid step supplied, do nothing
      return false;

    // Copy our parameters
    *par = m_parameters;

    // Work out which variable to change, and supply the correct variable
    double tmp =
      double (step) * (m_parameters.variableFinal -
                       m_parameters.variableStart) /
      double (m_parameters.variableSteps - 1) + m_parameters.variableStart;

    switch (m_parameters.variableWorking)
      {
      case e_voltageSource:
        par->voltageSource = tmp;
        break;
      case e_voltageDrain:
        par->voltageDrain = tmp;
        break;
      case e_defectPercentage:
        par->defectPercentage = tmp / 100.0;
        break;
      case e_trapPercentage:
        par->trapPercentage = tmp / 100.0;
        break;
      case e_chargePercentage:
        par->chargePercentage = tmp / 100.0;
        break;
      case e_temperatureKelvin:
        par->temperatureKelvin = tmp;
        break;
      default:
        return false;
      }
    return true;
  }

  inline void InputParser::processLine (QIODevice * file)
  {
    QString line = file->readLine ();
    if (line.at (0) == '#')
      return;

    QStringList list =
      line.split ("#", QString::SkipEmptyParts).at (0).split ('=',
                                                              QString::
                                                              SkipEmptyParts);

    if (list.size () == 2)
      {

        QString key = list.at (0).toLower ().trimmed ();

        switch (s_variables.value (key))
          {

          case e_voltageSource:
            {
              m_parameters.voltageSource = list.at (1).toDouble ();
              break;
            }

          case e_voltageDrain:
            {
              m_parameters.voltageDrain = list.at (1).toDouble ();
              break;
            }

          case e_defectPercentage:
            {
              m_parameters.defectPercentage = list.at (1).toDouble () / 100;
              if (m_parameters.defectPercentage < 0.00)
                //|| m_parameters.defectPercentage >
                //(1.00 - trapPercentage ()))
                {
                  qDebug () << "Defect percentage out of range:" <<
                    m_parameters.defectPercentage *
                    100.0 << " (0.00 -- 100.00)";
                  qFatal ("bad input");
                }
              break;
            }

          case e_trapPercentage:
            {
              m_parameters.trapPercentage = list.at (1).toDouble () / 100.0;
              if (m_parameters.trapPercentage < 0.00)
                //|| m_parameters.trapPercentage >
                //(1.00 - defectPercentage ()))
                {
                  qDebug () << "Trap percentage out of range:" <<
                    m_parameters.trapPercentage *
                    100.0 << " (0.00 -- 100.00)";
                  qFatal ("bad input");
                }
              break;
            }

          case e_chargePercentage:
            {
              m_parameters.chargePercentage = list.at (1).toDouble () / 100.0;
              if (m_parameters.chargePercentage < 0.00
                  || m_parameters.chargePercentage > 1.00)
                {
                  qDebug () << "Charge percentage out of range:" <<
                    m_parameters.chargePercentage *
                    100.0 << " (0.00 -- 100.00)";
                  qFatal ("bad input");
                }
              break;
            }

          case e_temperatureKelvin:
            {
              m_parameters.temperatureKelvin = list.at (1).toDouble ();
              if (m_parameters.temperatureKelvin <= 0)
                {
                  qDebug () << "Absolute temperature must be > 0K";
                  qFatal ("bad input");
                }
              break;
            }

          case e_deltaEpsilon:
            {
              m_parameters.deltaEpsilon = list.at (1).toDouble ();
              break;
            }

          case e_variableWorking:
            {
              m_parameters.variableWorking =
                s_variables.value (list.at (1).toLower ().trimmed ());
              if (m_parameters.variableWorking == e_voltageSource ||
                  m_parameters.variableWorking == e_voltageDrain ||
                  m_parameters.variableWorking == e_defectPercentage ||
                  m_parameters.variableWorking == e_trapPercentage ||
                  m_parameters.variableWorking == e_chargePercentage ||
                  m_parameters.variableWorking == e_temperatureKelvin)
                {
                }
              else
                {
                  qDebug () << "Working variable set to an invalid type:"
                    << list.at (1).toLower ().trimmed ();
                  qFatal ("bad input");
                }
              break;
            }

          case e_variableStart:
            {
              m_parameters.variableStart = list.at (1).toDouble ();
              break;
            }

          case e_variableFinal:
            {
              m_parameters.variableFinal = list.at (1).toDouble ();
              break;
            }

          case e_variableSteps:
            {
              m_parameters.variableSteps = list.at (1).toInt ();
              if (m_parameters.variableSteps < 1)
                {
                  qDebug () << "Number of steps must be >=1.";
                  qFatal ("bad input");
                }
              break;
            }

          case e_gridWidth:
            {
              m_parameters.gridWidth = list.at (1).toInt ();
              if (m_parameters.gridWidth < 1)
                {
                  qDebug () << "Grid width must be >=1.";
                  qFatal ("bad input");
                }
              break;
            }

          case e_gridHeight:
            {
              m_parameters.gridHeight = list.at (1).toInt ();
              if (m_parameters.gridHeight < 1)
                {
                  qDebug () << "Grid height must be >=1.";
                  qFatal ("bad input");
                }
              break;
            }

          case e_gridDepth:
            {
              m_parameters.gridDepth = list.at (1).toInt ();
              if (m_parameters.gridDepth < 1)
                {
                  qDebug () << "Grid depth must be >=1.";
                  qFatal ("bad input");
                }
              break;
            }

          case e_zDefect:
            {
              m_parameters.zDefect = list.at (1).toInt ();
              if (m_parameters.zDefect < -1 || m_parameters.zDefect > 1)
                {
                  qDebug () << "Charge on defect must be +/- 1e";
                  qFatal ("bad input");
                }
              break;
            }

          case e_zTrap:
            {
              m_parameters.zTrap = list.at (1).toInt ();
              if (m_parameters.zTrap < -1 || m_parameters.zTrap > 1)
                {
                  qDebug () << "Charge on trap must be +/- 1e";
                  qFatal ("bad input");
                }
              break;
            }

          case e_gridCharge:
            {
              QString gridCharge = list.at (1).trimmed ().toLower ();
              if (gridCharge == "true")
                {
                  m_parameters.gridCharge = true;
                }
              else if (gridCharge == "false")
                {
                  m_parameters.gridCharge = false;
                }
              else
                {
                  qDebug () << "Charging the grid is either true or false:" <<
                    gridCharge;
                  qFatal ("bad input");
                }
              break;
            }

          case e_potentialForm:
            {
              QString potentialForm = list.at (1).trimmed ().toLower ();
              if (potentialForm == "linear")
                {
                  m_parameters.potentialForm = 0;
                }
              else
                {
                  qDebug () << "potential.form must be linear:" <<
                    m_parameters.potentialForm;
                  qFatal ("bad input");
                }
              break;
            }

          case e_iterationsWarmup:
            {
              m_parameters.iterationsWarmup = list.at (1).toInt ();
              if (m_parameters.iterationsWarmup < 0)
                {
                  qDebug () << "Warmup iterations must be >=0.";
                  qFatal ("bad input");
                }
              break;
            }

          case e_iterationsReal:
            {
              m_parameters.iterationsReal = list.at (1).toInt ();
              if (m_parameters.iterationsReal < 1)
                {
                  qDebug () << "Real iterations must be >=1.";
                  qFatal ("bad input");
                }
              break;
            }

          case e_iterationsPrint:
            {
              m_parameters.iterationsPrint = list.at (1).toInt ();
              if (m_parameters.iterationsPrint < 0)
                {
                  qDebug () << "Print iterations must be >=0.";
                  qFatal ("bad input");
                }
              break;
            }

          case e_coulombInteraction:
            {
              QString interaction = list.at (1).trimmed ().toLower ();
              if (interaction == "true")
                {
                  m_parameters.coulomb = true;
                }
              else if (interaction == "false")
                {
                  m_parameters.coulomb = false;
                }
              else
                {
                  qDebug () <<
                    "Coulomb interactions are either true or false:" <<
                    interaction;
                  qFatal ("bad input");
                }
              break;
            }

          case e_chargedDefects:
            {
              QString interaction = list.at (1).trimmed ().toLower ();
              if (interaction == "true")
                {
                  m_parameters.chargedDefects = true;
                }
              else if (interaction == "false")
                {
                  m_parameters.chargedDefects = false;
                }
              else
                {
                  qDebug () << "Charged defects are either true or false: ";
                  qFatal ("bad input");
                }
              break;
            }

          case e_chargedTraps:
            {
              QString interaction = list.at (1).trimmed ().toLower ();
              if (interaction == "true")
                {
                  m_parameters.chargedTraps = true;
                }
              else if (interaction == "false")
                {
                  m_parameters.chargedTraps = false;
                }
              else
                {
                  qDebug () << "Charged traps are either true or false: ";
                  qFatal ("bad input");
                }
              break;
            }

          case e_outputXyz:
            {
              QString interaction = list.at (1).trimmed ().toLower ();
              if (interaction == "true")
                {
                  m_parameters.outputXyz = true;
                }
              else if (interaction == "false")
                {
                  m_parameters.outputXyz = false;
                }
              else
                {
                  qDebug () << "output.xyz is either true or false: ";
                  qFatal ("bad input");
                }
              break;
            }

          case e_potentialPoint:
            {
              QStringList q =
                list.at (1).trimmed ().toLower ().remove ("(").
                remove (")").split (",");
              if (q.length () != 4)
                {
                  qDebug () << "Invalid potential.point specified: " << q;
                  qFatal ("bad input");
                }
              bool ok = true;
              double x = q[0].toDouble (&ok);
              if (!ok)
                {
                  qDebug () << "0 Invalid potential.point specified: " << q <<
                    ok;
                  qFatal ("bad input");
                }
              double y = q[1].toDouble (&ok);
              if (!ok)
                {
                  qDebug () << "1 Invalid potential.point specified: " << q <<
                    ok;
                  qFatal ("bad input");
                }
              double z = q[2].toDouble (&ok);
              if (!ok)
                {
                  qDebug () << "2 Invalid potential.point specified: " << q <<
                    ok;
                  qFatal ("bad input");
                }
              double V = q[3].toDouble (&ok);
              if (!ok)
                {
                  qDebug () << "3 Invalid potential.point specified: " << q <<
                    ok;
                  qFatal ("bad input");
                }
              m_parameters.
                potentialPoints.push_back (PotentialPoint (x, y, z, V));
              break;
            }

          case e_gaussianStdev:
            {
              m_parameters.gaussianStdev = list.at (1).toDouble ();
              if (m_parameters.gaussianStdev <= 0.00)
                {
                  qDebug () <<
                    "Negative or zero standard deviation specified for random noise";
                  qFatal ("bad input");
                }
              break;
            }

          case e_gaussianAverg:
            {
              m_parameters.gaussianAverg = list.at (1).toDouble ();
              break;
            }

          case e_gaussianNoise:
            {
              QString choice = list.at (1).trimmed ().toLower ();
              if (choice == "true")
                {
                  m_parameters.gaussianNoise = true;
                }
              else if (choice == "false")
                {
                  m_parameters.gaussianNoise = false;
                }
              else
                {
                  qDebug () << "potential.noise is either true or false: ";
                  qFatal ("bad input");
                }
              break;
            }

          case e_trapsHeterogeneous:
            {
              QString trapsHeterogeneous = list.at (1).trimmed ().toLower ();
              if (trapsHeterogeneous == "true")
                {
                  m_parameters.trapsHeterogeneous = true;
                }
              else if (trapsHeterogeneous == "false")
                {
                  m_parameters.trapsHeterogeneous = false;
                }
              else
                {
                  qDebug () <<
                    "Heterogeneous Traps are either true or false: ";
                  qFatal ("bad input");
                }
              break;
            }

          case e_seedPercentage:
            {
              m_parameters.seedPercentage = list.at (1).toDouble () / 100.0;
              if (m_parameters.seedPercentage < 0.00
                  || m_parameters.seedPercentage >
                  m_parameters.trapPercentage)
                {
                  qDebug () << "Seed percentage out of range:" <<
                    m_parameters.seedPercentage *
                    100.0 << " (0.00 -- 100.00)";
                  qFatal ("bad input");
                }
              break;
            }


          case e_sourceBarrier:
            {
              m_parameters.sourceBarrier = list.at (1).toDouble () / 100.0;
              if (m_parameters.sourceBarrier <= 0.00
                  || m_parameters.sourceBarrier > 1.00)
                {
                  qDebug () << "Source injection probability out of range:" <<
                    m_parameters.defectPercentage << " (0.00 -- 100.00)";
                  qFatal ("bad input");
                }
              break;
            }

          case e_outputPrecision:
            {
              m_parameters.outputPrecision = list.at (1).toInt ();
              if (m_parameters.outputPrecision <= 0)
                {
                  qDebug () << "output precision must be >0.";
                  qFatal ("bad input");
                }
              break;
            }

          case e_outputWidth:
            {
              m_parameters.outputWidth = list.at (1).toInt ();
              if (m_parameters.outputWidth <= 0)
                {
                  qDebug () << "output width must be >0.";
                  qFatal ("bad input");
                }
              break;
            }

          case e_sourceType:
            {
              QString type = list.at (1).trimmed ().toLower ();
              if (type == "constant")
                {
                  m_parameters.sourceType = 0;
                }
              else if (type == "coulomb")
                {
                  m_parameters.sourceType = 1;
                }
              else
                {
                  qDebug () <<
                    "options for source barrier calculation type are constant and coulomb";
                  qFatal ("bad input");
                }
              break;
            }

          default:
            qDebug () << "Unknown key value encountered:" << key;
          }
      }
  }

  QMap < QString, InputParser::VariableType > InputParser::s_variables;

  void InputParser::initializeVariables ()
  {
    s_variables["interaction.coulomb"] = e_coulombInteraction;
    s_variables["charged.defects"] = e_chargedDefects;
    s_variables["gaussian.noise"] = e_gaussianNoise;
    s_variables["grid.charge"] = e_gridCharge;
    s_variables["output.xyz"] = e_outputXyz;
    s_variables["charged.traps"] = e_chargedTraps;
    s_variables["traps.heterogeneous"] = e_trapsHeterogeneous;

    s_variables["charge.percentage"] = e_chargePercentage;
    s_variables["defect.percentage"] = e_defectPercentage;
    s_variables["delta.epsilon"] = e_deltaEpsilon;
    s_variables["gaussian.averg"] = e_gaussianAverg;
    s_variables["gaussian.stdev"] = e_gaussianStdev;
    s_variables["seed.percentage"] = e_seedPercentage;
    s_variables["source.barrier"] = e_sourceBarrier;
    s_variables["temperature.kelvin"] = e_temperatureKelvin;
    s_variables["trap.percentage"] = e_trapPercentage;
    s_variables["variable.final"] = e_variableFinal;
    s_variables["variable.start"] = e_variableStart;
    s_variables["voltage.drain"] = e_voltageDrain;
    s_variables["voltage.source"] = e_voltageSource;

    s_variables["grid.depth"] = e_gridDepth;
    s_variables["grid.height"] = e_gridHeight;
    s_variables["grid.width"] = e_gridWidth;
    s_variables["iterations.print"] = e_iterationsPrint;
    s_variables["iterations.real"] = e_iterationsReal;
    s_variables["iterations.warmup"] = e_iterationsWarmup;
    s_variables["output.precision"] = e_outputPrecision;
    s_variables["output.width"] = e_outputWidth;
    s_variables["potential.form"] = e_potentialForm;
    s_variables["source.type"] = e_sourceType;
    s_variables["variable.steps"] = e_variableSteps;
    s_variables["variable.working"] = e_variableWorking;
    s_variables["z.defect"] = e_zDefect;
    s_variables["z.trap"] = e_zTrap;
    s_variables["potential.point"] = e_potentialPoint;

  }
}
