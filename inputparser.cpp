#include "inputparser.h"
#include "simulation.h"

#include <QtCore/QFile>
#include <QtCore/QIODevice>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QDebug>

namespace Langmuir {

  InputParser::InputParser(const QString& fileName)
      : m_varyType(e_undefined), m_start(0.0), m_final(0.0), m_steps(1),
      m_valid(true)
  {
    if (s_variables.isEmpty())
      initializeVariables();

    // Open up this input file
    QFile* file = new QFile(fileName);
    if (!file->open(QIODevice::ReadOnly | QIODevice::Text)) {
      qDebug() << "Error opening file:" << fileName;
      delete file;
      file = 0;
      return;
    }

    // Now read the input file in, line by line
    while (!file->atEnd())
      processLine(file);

  }

  bool InputParser::simulationParameters(SimulationParameters *par, int step)
  {
    if (step < 0 || step >= m_steps) // Invalid step supplied, do nothing
      return false;

    // Copy our parameters
    *par = m_parameters;

    // Work out which variable to change, and supply the correct variable
    double tmp = double(step) * (m_final - m_start) / double(m_steps-1)
                 + m_start;

    switch (m_varyType) {
      case e_voltageSource:
        par->voltageSource = tmp;
        break;
      case e_voltageDrain:
        par->voltageDrain = tmp;
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
      default: // This should not happen - error...
        m_valid = false;
        return false;
    }
    return true;
  }

  QString InputParser::workingVariable() const
  {
    switch (m_varyType) {
      case e_voltageSource:
        return "voltage.source";
      case e_voltageDrain:
        return "voltage.drain";
      case e_trapPercentage:
        return "trap.percentage";
      case e_chargePercentage:
        return "charge.percentage";
      case e_temperatureKelvin:
        return "temperature.kelvin";
      default: // This should not happen - error...
        return "error.undefined";
    }
  }

  inline void InputParser::processLine(QIODevice *file)
  {
    QString line = file->readLine();
    // If the line starts with a '#' then it is a comment
    if (line.at(0) == '#')
      return;

    qDebug() << line.trimmed();
    QStringList list = line.split('=', QString::SkipEmptyParts);
    if (list.size() == 2) { // We have a key value pair
      QString key = list.at(0).toLower().trimmed();
      switch (s_variables.value(key)) {
        case e_voltageSource: {
          m_parameters.voltageSource = list.at(1).toDouble();
          qDebug() << "Source voltage:" << m_parameters.voltageSource;
          break;
        }
        case e_voltageDrain: {
          m_parameters.voltageDrain = list.at(1).toDouble();
          qDebug() << "Drain voltage:" << m_parameters.voltageDrain;
          break;
        }
        case e_trapPercentage: {
          m_parameters.trapPercentage = list.at(1).toDouble() / 100.0;
          if (m_parameters.trapPercentage < 0.00 ||
              m_parameters.trapPercentage > 1.00) {
            m_valid = false;
            qDebug() << "Trap percentage out of range:"
                << m_parameters.trapPercentage*100.0 << "(0.00 -- 100.00)";
          }
          qDebug() << "Trap percentage:" << m_parameters.trapPercentage;
          break;
        }
        case e_chargePercentage: {
          m_parameters.chargePercentage = list.at(1).toDouble() / 100.0;
          if (m_parameters.chargePercentage < 0.00 ||
              m_parameters.chargePercentage > 1.00) {
            m_valid = false;
            qDebug() << "Charge percentage out of range:"
                << m_parameters.chargePercentage*100.0 << "(0.00 -- 100.00)";
          }
          qDebug() << "Charge percentage:" << m_parameters.chargePercentage;
          break;
        }
        case e_temperatureKelvin: {
          m_parameters.temperatureKelvin = list.at(1).toDouble();
          // The temperature cannot be lower than 0K
          if (m_parameters.temperatureKelvin < 0.00) {
            m_valid = false;
          }
          break;   
        }   
        case e_variableWorking: {
          m_varyType = s_variables.value(list.at(1).toLower().trimmed());
          // Check that the working variable is a valid one
          if (m_varyType == e_voltageSource ||
              m_varyType == e_voltageDrain ||
              m_varyType == e_trapPercentage ||
              m_varyType == e_chargePercentage ||
              m_varyType == e_temperatureKelvin) {
            qDebug() << "Working variable:" << m_varyType;
          }
          else {
            m_valid = false;
            qDebug() << "Working variable set to an invalid type:"
                << list.at(1).toLower().trimmed();
          }
          break;
        }
        case e_variableStart: {
          m_start = list.at(1).toDouble();
          qDebug() << "variable.start:" << m_start;
          break;
        }
        case e_variableFinal: {
          m_final = list.at(1).toDouble();
          qDebug() << "variable.final:" << m_final;
          break;
        }
        case e_variableSteps: {
          m_steps = list.at(1).toInt();
          if (m_steps < 1) {
            m_valid = false;
            qDebug() << "Number of steps must be >=1.";
          }
          qDebug() << "variable.steps:" << m_steps;
          break;
        }
        case e_gridWidth: {
          m_parameters.gridWidth = list.at(1).toInt();
          if (m_parameters.gridWidth < 1) {
            m_valid = false;
            qDebug() << "Grid width must be >=1.";
          }
          qDebug() << "grid.width:" << m_parameters.gridWidth;
          break;
        }
        case e_gridHeight: {
          m_parameters.gridHeight = list.at(1).toInt();
          if (m_parameters.gridHeight < 1) {
            m_valid = false;
            qDebug() << "Grid height must be >=1.";
          }
          qDebug() << "grid.height:" << m_parameters.gridHeight;
          break;
        }
		case e_zDefect:  {
			m_parameters.zDefect=list.at(1).toInt();
			if (m_parameters.zDefect < -1 || m_parameters.zDefect > 1) {
			m_valid = false;
			}
			break;
		}
        case e_gridCharge: {
          QString gridCharge = list.at(1).trimmed().toLower();
          if (gridCharge == "true")
            m_parameters.gridCharge = true;
          else if (gridCharge == "false")
            m_parameters.gridCharge = false;
          else {
            m_valid = false;
            qDebug() << "Charging the grid is either true or false:"
                << gridCharge;
          }
          qDebug() << "grid.charge:" << m_parameters.gridCharge;
          break;
        }
        case e_iterationsWarmup: {
          m_parameters.iterationsWarmup = list.at(1).toInt();
          if (m_parameters.iterationsWarmup < 0) {
            m_valid = false;
            qDebug() << "Warmup iterations must be >=0.";
          }
          qDebug() << "iterations.warmup:" << m_parameters.iterationsWarmup;
          break;
        }
        case e_iterationsReal: {
          m_parameters.iterationsReal = list.at(1).toInt();
          if (m_parameters.iterationsReal < 1) {
            m_valid = false;
            qDebug() << "Real iterations must be >=1.";
          }
          qDebug() << "iterations.real:" << m_parameters.iterationsReal;
          break;
        }
        case e_iterationsPrint: {
          m_parameters.iterationsPrint = list.at(1).toInt();
          if (m_parameters.iterationsPrint < 0) {
            m_valid = false;
            qDebug() << "Print iterations must be >=0.";
          }
          qDebug() << "iterations.print:" << m_parameters.iterationsPrint;
          break;
        }
        case e_coulombInteraction: {
          QString interaction = list.at(1).trimmed().toLower();
          if (interaction == "true")
            m_parameters.coulomb = true;
          else if (interaction == "false")
            m_parameters.coulomb = false;
          else {
            m_valid = false;
            qDebug() << "Coulomb interactions are either true or false:"
                << interaction;
          }
          qDebug() << "interaction.coulomb:" << m_parameters.coulomb;
          break;
        }
		case e_defectsCharged:  {
		  QString interaction = list.at(1).trimmed().toLower();
		  if (interaction == "true")
			  m_parameters.defectsCharged = true;
		  else if (interaction == "false")
			  m_parameters.defectsCharged = false;
		  else {
			  m_valid = false;
		  }
		  break;
		}

			
        default:
          qDebug() << "Unknown key value encountered:" << key;
      }
    }

  }

  QMap<QString, InputParser::VariableType> InputParser::s_variables;

  void InputParser::initializeVariables()
  {
    // Initializes the static string to enum map necessary for input parsing
    s_variables["voltage.source"] = e_voltageSource;
    s_variables["voltage.drain"] = e_voltageDrain;
    s_variables["trap.percentage"] = e_trapPercentage;
    s_variables["charge.percentage"] = e_chargePercentage;
    s_variables["temperature.kelvin"] = e_temperatureKelvin;
    s_variables["variable.working"] = e_variableWorking;
    s_variables["variable.start"] = e_variableStart;
    s_variables["variable.final"] = e_variableFinal;
    s_variables["variable.steps"] = e_variableSteps;
    s_variables["grid.width"] = e_gridWidth;
    s_variables["grid.height"] = e_gridHeight;
    s_variables["grid.height"] = e_gridHeight;
	s_variables["z.defect"] = e_zDefect;
    s_variables["grid.charge"] = e_gridCharge;
    s_variables["iterations.warmup"] = e_iterationsWarmup;
    s_variables["iterations.real"] = e_iterationsReal;
    s_variables["iterations.print"] = e_iterationsPrint;
    s_variables["interaction.coulomb"] = e_coulombInteraction;
	s_variables["charged.defects"] = e_defectsCharged;
  }

}
