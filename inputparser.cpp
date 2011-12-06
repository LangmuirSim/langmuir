#include "inputparser.h"
#include "simulation.h"
#include <QtCore>

namespace Langmuir
{

  InputParser::InputParser (const QString & fileName)
  {
    if (s_variables.isEmpty ())
      initializeVariables ();

    readCount = 0;

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

    //fix errors and parameter conflicts
    if (m_parameters.outputWidth < (m_parameters.outputPrecision + 15))
      {
        m_parameters.outputWidth = m_parameters.outputPrecision + 15;
      }
    if (m_parameters.outputWidth < 20)
      {
        m_parameters.outputWidth = 20;
      }
    if (m_parameters.defectPercentage > 1.00 - m_parameters.trapPercentage)
      qFatal ("percent defects + percent traps > 100 percent");

    m_parameters.inverseKT =
      1.0 / (m_parameters.boltzmannConstant * m_parameters.temperatureKelvin);

    m_parameters.electrostaticPrefactor =
      m_parameters.elementaryCharge / (4.0 * M_PI *
                                       m_parameters.dielectricConstant *
                                       m_parameters.permittivitySpace *
                                       m_parameters.gridFactor);

  }

  bool InputParser::simulationParameters (SimulationParameters * par,
                                          int step )
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
        {
          par->voltageSource = tmp;
          break;
        }

      case e_voltageDrain:
        {
          par->voltageDrain = tmp;
          break;
        }

      case e_defectPercentage:
        {
          par->defectPercentage = tmp / 100.0;
          if (par->defectPercentage < 0 || par->defectPercentage > 1.00)
            {
              qFatal
                ("percent defects specified by working variable out of range");
            }
          if (par->defectPercentage > 1.00 - par->trapPercentage)
            {
              qFatal
                ("percent defects + percent traps > 100 percent specified by working variable");
            }
          break;
        }

      case e_trapPercentage:
        {
          par->trapPercentage = tmp / 100.0;
          if (par->trapPercentage < 0 || par->trapPercentage > 1.00)
            {
              qFatal
                ("percent defects specified by working variable out of range");
            }
          if (par->defectPercentage > 1.00 - par->trapPercentage)
            {
              qFatal
                ("percent defects + percent traps > 100 percent specified by working variable");
            }
          break;
        }

      case e_chargePercentage:
        {
          par->chargePercentage = tmp / 100.0;
          if (par->chargePercentage < 0 || par->chargePercentage > 1.00)
            qFatal
              ("percent defects specified by working variable out of range");
          break;
        }

      case e_temperatureKelvin:
        {
          par->temperatureKelvin = tmp;
          if (par->temperatureKelvin <= 0)
            {
              qFatal
                ("zero or negative temperature specified by working variable");
            }
          par->inverseKT =
            1.0 / (par->boltzmannConstant * par->temperatureKelvin);
          break;
        }

      default:
        {
          return false;
        }

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
                                                              QString::SkipEmptyParts);

    if (list.size () == 2)
      {

        QString key = list.at (0).toLower ().trimmed ();

        switch (s_variables.value (key))
          {

          case e_voltageSource:
            {
              m_parameters.voltageSource = list.at (1).toDouble ();
              readCount += 1;
              break;
            }

          case e_voltageDrain:
            {
              m_parameters.voltageDrain = list.at (1).toDouble ();
              readCount += 1;
              break;
            }

          case e_defectPercentage:
            {
              m_parameters.defectPercentage = list.at (1).toDouble () / 100;
              if (m_parameters.defectPercentage < 0.00 || m_parameters.defectPercentage > 1.00)
                {
                  qDebug () << "Defect percentage out of range:" <<
                    m_parameters.defectPercentage *
                    100.0 << " (0.00 -- 100.00)";
                  qFatal ("bad input");
                }
              readCount += 1;
              break;
            }

          case e_trapPercentage:
            {
              m_parameters.trapPercentage = list.at (1).toDouble () / 100.0;
              if (m_parameters.trapPercentage < 0.00 || m_parameters.trapPercentage > 1.00)
                {
                  qDebug () << "Trap percentage out of range:" <<
                    m_parameters.trapPercentage *
                    100.0 << " (0.00 -- 100.00)";
                  qFatal ("bad input");
                }
              readCount += 1;
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
              readCount += 1;
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
              m_parameters.inverseKT =
                1.0 / (m_parameters.boltzmannConstant *
                       m_parameters.temperatureKelvin);
              readCount += 1;
              break;
            }

          case e_deltaEpsilon:
            {
              m_parameters.deltaEpsilon = list.at (1).toDouble ();
              readCount += 1;
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
              readCount += 1;
              break;
            }

          case e_variableStart:
            {
              m_parameters.variableStart = list.at (1).toDouble ();
              readCount += 1;
              break;
            }

          case e_variableFinal:
            {
              m_parameters.variableFinal = list.at (1).toDouble ();
              readCount += 1;
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
              readCount += 1;
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
              readCount += 1;
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
              readCount += 1;
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
              readCount += 1;
              break;
            }

          case e_zDefect:
            {
              m_parameters.zDefect = list.at (1).toInt ();
              readCount += 1;
              break;
            }

          case e_zTrap:
            {
              m_parameters.zTrap = list.at (1).toInt ();
              readCount += 1;
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
              readCount += 1;
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
              readCount += 1;
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
              readCount += 1;
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
              readCount += 1;
              break;
            }

          case e_interactionCoulomb:
            {
              QString interaction = list.at (1).trimmed ().toLower ();
              if (interaction == "true")
                {
                  m_parameters.interactionCoulomb = true;
                }
              else if (interaction == "false")
                {
                  m_parameters.interactionCoulomb = false;
                }
              else
                {
                  qDebug () <<
                    "Coulomb interactions are either true or false:" <<
                    interaction;
                  qFatal ("bad input");
                }
              readCount += 1;
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
              readCount += 1;
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
              readCount += 1;
              break;
            }

          case e_outputGrid:
          {
              QString interaction = list.at (1).trimmed ().toLower ();
              if (interaction == "true")
              {
                  m_parameters.outputGrid = true;
              }
              else if (interaction == "false")
              {
                  m_parameters.outputGrid = false;
              }
              else
              {
                  qDebug () << "output.grid is either true or false: ";
                  qFatal ("bad input");
              }
              readCount += 1;
              break;
          }

          case e_outputCoulombPotential:
          {
              QString interaction = list.at (1).trimmed ().toLower ();
              if (interaction == "true")
              {
                  m_parameters.outputCoulombPotential = true;
              }
              else if (interaction == "false")
              {
                  m_parameters.outputCoulombPotential = false;
              }
              else
              {
                  qDebug () << "output.coulombPotential is either true or false: ";
                  qFatal ("bad input");
              }
              readCount += 1;
              break;
          }

          case e_outputCarriers:
          {
              QString interaction = list.at (1).trimmed ().toLower ();
              if (interaction == "true")
              {
                  m_parameters.outputCarriers = true;
              }
              else if (interaction == "false")
              {
                  m_parameters.outputCarriers = false;
              }
              else
              {
                  qDebug () << "output.carriers is either true or false: ";
                  qFatal ("bad input");
              }
              readCount += 1;
              break;
          }

          case e_outputFieldPotential:
          {
              QString interaction = list.at (1).trimmed ().toLower ();
              if (interaction == "true")
              {
                  m_parameters.outputFieldPotential = true;
              }
              else if (interaction == "false")
              {
                  m_parameters.outputFieldPotential = false;
              }
              else
              {
                  qDebug () << "output.fieldPotential is either true or false: ";
                  qFatal ("bad input");
              }
              readCount += 1;
              break;
          }

          case e_outputTrapIDs:
          {
              QString interaction = list.at (1).trimmed ().toLower ();
              if (interaction == "true")
              {
                  m_parameters.outputTrapIDs = true;
              }
              else if (interaction == "false")
              {
                  m_parameters.outputTrapIDs = false;
              }
              else
              {
                  qDebug () << "output.trapIDs is either true or false: ";
                  qFatal ("bad input");
              }
              readCount += 1;
              break;
          }

          case e_outputDefectIDs:
          {
              QString interaction = list.at (1).trimmed ().toLower ();
              if (interaction == "true")
              {
                  m_parameters.outputDefectIDs = true;
              }
              else if (interaction == "false")
              {
                  m_parameters.outputDefectIDs = false;
              }
              else
              {
                  qDebug () << "output.defectIDs is either true or false: ";
                  qFatal ("bad input");
              }
              readCount += 1;
              break;
          }

          case e_potentialLinear:
          {
              QString interaction = list.at (1).trimmed ().toLower ();
              if (interaction == "true")
              {
                  m_parameters.potentialLinear = true;
              }
              else if (interaction == "false")
              {
                  m_parameters.potentialLinear = false;
              }
              else
              {
                  qDebug () << "output.potentialLinear is either true or false: ";
                  qFatal ("bad input");
              }
              readCount += 1;
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
              readCount += 1;
              break;
            }

          case e_gaussianAverg:
            {
              m_parameters.gaussianAverg = list.at (1).toDouble ();
              readCount += 1;
              break;
            }

          case e_seedPercentage:
            {
              m_parameters.seedPercentage = list.at (1).toDouble () / 100.0;
              if (m_parameters.seedPercentage < 0.00
                  || m_parameters.seedPercentage >
                  1.00)
                {
                  qDebug () << "Seed percentage out of range:" <<
                    m_parameters.seedPercentage *
                    100.0 << " (0.00 -- 100.00)";
                  qFatal ("bad input");
                }
              readCount += 1;
              break;
            }

          case e_sourceBarrier:
            {
              m_parameters.sourceBarrier = list.at (1).toDouble () / 100.0;
              if (m_parameters.sourceBarrier <= 0.00
                  || m_parameters.sourceBarrier > 1.00)
                {
                  qDebug () << "Source injection probability out of range:" <<
                    m_parameters.sourceBarrier << " (0.00 -- 100.00)";
                  qFatal ("bad input");
                }
              readCount += 1;
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
              readCount += 1;
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
              readCount += 1;
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
              else if (type == "image")
                {
                  m_parameters.sourceType = 2;
                }
              else
                {
                  qDebug () <<
                    "options for source barrier calculation type are constant, coulomb, and image";
                  qFatal ("bad input");
                }
              readCount += 1;
              break;
            }

          case e_hoppingRange:
            {
              m_parameters.hoppingRange = list.at (1).toInt ();
              if (m_parameters.hoppingRange < 1)
                {
                  qDebug () << "hopping range must be > 1.";
                  qFatal ("bad input");
                }
              readCount += 1;
              break;
            }

          case e_useOpenCL:
            {
              QString useOpenCL = list.at (1).trimmed ().toLower ();
              if (useOpenCL == "true")
                {
                  m_parameters.useOpenCL = true;
                }
              else if (useOpenCL == "false")
                {
                  m_parameters.useOpenCL = false;
                }
              else
                {
                  qDebug () << "OpenCL is either true or false:" <<
                    useOpenCL;
                  qFatal ("bad input");
                }
              readCount += 1;
              break;
            }

          case e_workWidth:
            {
              m_parameters.workWidth = list.at (1).toInt ();
              if (m_parameters.workWidth <= 0)
                {
                  qDebug () << "Local work group size width be larger than 0.";
                  qFatal ("bad input");
                }
              readCount += 1;
              break;
            }

          case e_workHeight:
            {
              m_parameters.workHeight = list.at (1).toInt ();
              if (m_parameters.workHeight <= 0)
                {
                  qDebug () << "Local work group height must be larger than 0.";
                  qFatal ("bad input");
                }
              readCount += 1;
              break;
            }

          case e_workDepth:
            {
              m_parameters.workDepth = list.at (1).toInt ();
              if (m_parameters.workDepth <= 0)
                {
                  qDebug () << "Local work group depth must be larger than 0.";
                  qFatal ("bad input");
                }
              readCount += 1;
              break;
            }

          case e_workSize:
            {
              m_parameters.workSize = list.at (1).toInt ();
              if (m_parameters.workSize <= 0)
                {
                  qDebug () << "Local work group size must be larger than 0.";
                  qFatal ("bad input");
                }
              readCount += 1;
              break;
            }

          case e_kernelsPath:
            {
              m_parameters.kernelsPath = list.at (1).trimmed ();
              readCount += 1;
              break;
            }

          case e_randomSeed:
            {
              m_parameters.randomSeed = list.at (1).toInt ();
              readCount += 1;
              break;
            }

          case e_outputStats:
          {
              QString interaction = list.at (1).trimmed ().toLower ();
              if (interaction == "true")
              {
                  m_parameters.outputStats = true;
              }
              else if (interaction == "false")
              {
                  m_parameters.outputStats = false;
              }
              else
              {
                  qDebug () << "output.stats is either true or false: ";
                  qFatal ("bad input");
              }
              readCount += 1;
              break;
          }

          case e_sourceAttempts:
          {
              QString type = list.at (1).trimmed ().toLower ();
              if (type == "max")
              {
                  m_parameters.sourceAttempts = -1;
              }
              else
              {
                 bool ok = true;
                 m_parameters.sourceAttempts = type.toInt(&ok);

                 if ( !ok )
                 {
                     qDebug () << "error reading source.attempts";
                     qFatal ("bad input");
                 }
              }
              readCount += 1;
              break;
          }

          case e_drainType:
            {
              QString type = list.at (1).trimmed ().toLower ();
              if (type == "constant")
                {
                  m_parameters.drainType = 0;
                }
              else if (type == "broke")
                {
                  m_parameters.drainType = 1;
                }
              else
                {
                  qDebug () <<
                    "options for drain barrier calculation type are constant and broke";
                  qFatal ("bad input");
                }
              readCount += 1;
              break;
            }

          case e_drainBarrier:
            {
              m_parameters.drainBarrier = list.at (1).toDouble () / 100.0;
              if (m_parameters.drainBarrier <= 0.00
                  || m_parameters.drainBarrier > 1.00)
                {
                  qDebug () << "Drain rejection probability out of range:" <<
                    m_parameters.drainBarrier << " (0.00 -- 100.00)";
                  qFatal ("bad input");
                }
              readCount += 1;
              break;
            }

          default:
            qDebug () << "Unknown key value encountered:" << qPrintable( line.trimmed() );
            break;
          }
      }
  }

  QMap < QString, InputParser::VariableType > InputParser::s_variables;

  void InputParser::initializeVariables ()
  {
    s_variables["boltzmaan.constant"] = e_boltzmannConstant;
    s_variables["charge.percentage"] = e_chargePercentage;
    s_variables["charged.defects"] = e_chargedDefects;
    s_variables["charged.traps"] = e_chargedTraps;
    s_variables["defect.percentage"] = e_defectPercentage;
    s_variables["delta.epsilon"] = e_deltaEpsilon;
    s_variables["dielectric.constant"] = e_dielectricConstant;
    s_variables["drain.barrier"] = e_drainBarrier;
    s_variables["drain.type"] = e_drainType;
    s_variables["electrostatic.cutoff"] = e_electrostaticCutoff;
    s_variables["electrostatic.prefactor"] = e_electrostaticPrefactor;
    s_variables["elementary.charge"] = e_elementaryCharge;
    s_variables["gaussian.averg"] = e_gaussianAverg;
    s_variables["gaussian.stdev"] = e_gaussianStdev;
    s_variables["grid.charge"] = e_gridCharge;
    s_variables["grid.depth"] = e_gridDepth;
    s_variables["grid.factor"] = e_gridFactor;
    s_variables["grid.height"] = e_gridHeight;
    s_variables["grid.width"] = e_gridWidth;
    s_variables["hopping.range"] = e_hoppingRange;
    s_variables["interaction.coulomb"] = e_interactionCoulomb;
    s_variables["inverse.KT"] = e_inverseKT;
    s_variables["iterations.print"] = e_iterationsPrint;
    s_variables["iterations.real"] = e_iterationsReal;
    s_variables["iterations.warmup"] = e_iterationsWarmup;
    s_variables["kernels.path"] = e_kernelsPath;
    s_variables["use.opencl"] = e_useOpenCL;
    s_variables["output.grid"] = e_outputGrid;
    s_variables["output.precision"] = e_outputPrecision;
    s_variables["output.stats"] = e_outputStats;
    s_variables["output.width"] = e_outputWidth;
    s_variables["output.xyz"] = e_outputXyz;
    s_variables["output.carriers"] = e_outputCarriers;
    s_variables["output.coulombpotential"] = e_outputCoulombPotential;
    s_variables["output.fieldpotential"] = e_outputFieldPotential;
    s_variables["output.trappotential"] = e_outputTrapPotential;
    s_variables["output.trapids"] = e_outputTrapIDs;
    s_variables["output.defectids"] = e_outputDefectIDs;
    s_variables["permittivity.space"] = e_permittivitySpace;
    s_variables["random.seed"] = e_randomSeed;
    s_variables["seed.percentage"] = e_seedPercentage;
    s_variables["source.attempts"] = e_sourceAttempts;
    s_variables["source.barrier"] = e_sourceBarrier;
    s_variables["source.type"] = e_sourceType;
    s_variables["temperature.kelvin"] = e_temperatureKelvin;
    s_variables["trap.percentage"] = e_trapPercentage;
    s_variables["variable.final"] = e_variableFinal;
    s_variables["variable.start"] = e_variableStart;
    s_variables["variable.steps"] = e_variableSteps;
    s_variables["variable.working"] = e_variableWorking;
    s_variables["voltage.drain"] = e_voltageDrain;
    s_variables["voltage.source"] = e_voltageSource;
    s_variables["work.width"] = e_workWidth;
    s_variables["work.height"] = e_workHeight;
    s_variables["work.depth"] = e_workDepth;
    s_variables["work.size"] = e_workSize;
    s_variables["z.defect"] = e_zDefect;
    s_variables["z.trap"] = e_zTrap;
    s_variables["potential.linear"] = e_potentialLinear;
  }

  int InputParser::getReadCount()
  {
      return readCount;
  }

  InputParserTemp::InputParserTemp(const QString &fileName)
  {
      parseFile(fileName);
  }

  void InputParserTemp::parseFile(const QString &fileName)
  {
      m_map.clear( );
      QFile     handle( fileName );
      QFileInfo   info( fileName );

      if ( ! handle.open ( QIODevice::ReadOnly | QIODevice::Text ) )
          qFatal ( "can not open input file: %s", qPrintable ( fileName ) );

      QRegExp regex1("\\s*=\\s*"  );
      QRegExp regex2("\\s*,\\s*"  );
      QRegExp regex3("\\s*\\[\\s*");
      QRegExp regex4("\\s*\\]\\s*");
      QRegExp regex5("#.*$"       );

      int lineNumber = 0;
      while ( !handle.atEnd() )
      {
          QString unaltered = handle.readLine( ).trimmed();
          QString line = unaltered;
          line = line.replace( regex1, "=" );
          line = line.replace( regex2, "," );
          line = line.replace( regex3,  "" );
          line = line.replace( regex4,  "" );
          line = line.replace( regex5,  "" );
          line = line.toLower( );
          line = line.trimmed( );

          if ( line.length() > 0 )
          {
              if ( line.count("=") != 1 )
              {
                  qFatal("InputParser syntax error:\n"
                         " file: %s\n"
                         " line: %d\n"
                         " text: %s\n"
                         " what: line must contain 1 equal sign.",
                         qPrintable(info.fileName()),lineNumber,qPrintable(unaltered));
              }

              QStringList tokens = line.split ("=",QString::SkipEmptyParts);

              if ( tokens.size() != 2 )
              {
                  qFatal("InputParser syntax error:\n"
                         " file: %s\n"
                         " line: %d\n"
                         " text: %s\n"
                         " what: line does not split into 2 sections at the equal sign.",
                         qPrintable(info.fileName()),lineNumber,qPrintable(unaltered));
              }

              QString key = tokens[0].trimmed();

              if ( key.isEmpty() )
              {
                  qFatal("InputParser syntax error:\n"
                         " file: %s\n"
                         " line: %d\n"
                         " text: %s\n"
                         " what: key is an empty string",
                         qPrintable(info.fileName()),lineNumber,qPrintable(unaltered));
              }

              QStringList values = tokens[1].split(",",QString::SkipEmptyParts);

              if ( values.size() == 0 )
              {
                  qFatal("InputParser syntax error:\n"
                         " file: %s\n"
                         " line: %d\n"
                         " text: %s\n"
                         "  key: %s\n"
                         " what: key specifies no values",
                         qPrintable(info.fileName()),lineNumber,qPrintable(unaltered),qPrintable(key));
              }

              for ( int i = 0; i < values.size(); i++ )
              {
                  QString value = values[i].trimmed();

                  if ( value.isEmpty() )
                  {
                      qFatal("InputParser syntax error:\n"
                             " file: %s\n"
                             " line: %d\n"
                             " text: %s\n"
                             "  key: %s\n"
                             " what: key value %d is an empty string",
                             qPrintable(info.fileName()),lineNumber,qPrintable(unaltered),qPrintable(key),i);
                  }

                  if ( ! m_map.contains(key) )
                  {
                      m_map[key] = QVector< QString >();
                  }

                  if ( m_map[key].contains( value ) )
                  {
                      qFatal("InputParser syntax error:\n"
                             " file: %s\n"
                             " line: %d\n"
                             " text: %s\n"
                             "  key: %s\n"
                             " what: duplicate value found",
                             qPrintable(info.fileName()),lineNumber,qPrintable(unaltered),qPrintable(key));
                  }
                  else
                  {
                      m_map[key].push_back(value);
                  }
              }
          }
          lineNumber += 1;
      }
      printMap();
  }

  void InputParserTemp::printMap()
  {
      for ( QMap<QString, QVector<QString> >::iterator i = m_map.begin();
            i != m_map.end(); i++ )
      {
          qDebug("%s",qPrintable( i.key() ) );
          for ( int j = 0; j < i.value().size(); j++ )
          {
              qDebug(" %s",qPrintable( i.value().at(j) ) );
          }
      }
  }

}
