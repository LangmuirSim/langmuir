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
      case e_defectPercentage:
        return "defect.percentage";
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

    //qDebug() << line.trimmed();
    QStringList list = line.split('=', QString::SkipEmptyParts);

    if (list.size() == 2) { // We have a key value pair

      QString key = list.at(0).toLower().trimmed();

      switch (s_variables.value(key)) {

       case e_voltageSource: {
        m_parameters.voltageSource = list.at(1).toDouble();
        //qDebug() << "Source voltage:" << m_parameters.voltageSource;
        break;
       }

       case e_voltageDrain: {
        m_parameters.voltageDrain = list.at(1).toDouble();
        //qDebug() << "Drain voltage:" << m_parameters.voltageDrain;
        break;
       }

       case e_defectPercentage: {
        m_parameters.defectPercentage = list.at(1).toDouble() / 100;
        if (m_parameters.defectPercentage < 0.00 || m_parameters.defectPercentage > (1.00 - trapPercentage())) {
         m_valid = false;
         qDebug() << "Defect percentage out of range:" <<  m_parameters.defectPercentage*100.0 << "(0.00 -- 100.00)";
         throw(std::invalid_argument("bad input"));
	}
        //qDebug()  << "Defect percentage:" << m_parameters.defectPercentage;
        break;
       }

       case e_trapPercentage: {
        m_parameters.trapPercentage = list.at(1).toDouble() / 100.0;
        if (m_parameters.trapPercentage < 0.00 || m_parameters.trapPercentage > (1.00 - defectPercentage())) {
         m_valid = false;
         qDebug() << "Trap percentage out of range:" <<  m_parameters.trapPercentage*100.0 << "(0.00 -- 100.00)";
         throw(std::invalid_argument("bad input"));
        }
        //qDebug() << "Trap percentage:" << m_parameters.trapPercentage;
        break;
       }

       case e_chargePercentage: {
        m_parameters.chargePercentage = list.at(1).toDouble() / 100.0;
        if (m_parameters.chargePercentage < 0.00 || m_parameters.chargePercentage > 1.00) {
         m_valid = false;
         qDebug() << "Charge percentage out of range:" << m_parameters.chargePercentage*100.0 << "(0.00 -- 100.00)";
         throw(std::invalid_argument("bad input"));
        }
        //qDebug() << "Charge percentage:" << m_parameters.chargePercentage;
        break;
       }

       case e_temperatureKelvin: {
        m_parameters.temperatureKelvin = list.at(1).toDouble();
        // The temperature cannot be lower than 0K
        if (m_parameters.temperatureKelvin < 0.00) {
         m_valid = false;
         qDebug() << "Absolute temperature must be < 0K";
         throw(std::invalid_argument("bad input"));
        }
        //qDebug() << "temperature.kelvin: " << m_parameters.temperatureKelvin; 
        break; 
       }

       case e_deltaEpsilon: {
        m_parameters.deltaEpsilon = list.at(1).toDouble();
        //qDebug() << "delta.epsilon: " << m_parameters.deltaEpsilon;
        break;
       }   

       case e_variableWorking: {
        m_varyType = s_variables.value(list.at(1).toLower().trimmed());
        // Check that the working variable is a valid one
        if ( m_varyType == e_voltageSource ||
             m_varyType == e_voltageDrain ||
             m_varyType == e_defectPercentage ||
             m_varyType == e_trapPercentage ||
             m_varyType == e_chargePercentage ||
             m_varyType == e_temperatureKelvin ) {
             //qDebug() << "Working variable:" << m_varyType;
        }
        else {
         m_valid = false;
         qDebug() << "Working variable set to an invalid type:"
                  << list.at(1).toLower().trimmed();
         throw(std::invalid_argument("bad input"));
        }
        break;
       }

       case e_variableStart: {
        m_start = list.at(1).toDouble();
        //qDebug() << "variable.start:" << m_start;
        break;
       }

       case e_variableFinal: {
        m_final = list.at(1).toDouble();
        //qDebug() << "variable.final:" << m_final;
        break;
       }

       case e_variableSteps: {
        m_steps = list.at(1).toInt();
        if (m_steps < 1) {
         m_valid = false;
         qDebug() << "Number of steps must be >=1.";
         throw(std::invalid_argument("bad input"));
        }
        //qDebug() << "variable.steps:" << m_steps;
        break;
       }

       case e_gridWidth: {
        m_parameters.gridWidth = list.at(1).toInt();
        if (m_parameters.gridWidth < 1) {
         m_valid = false;
         qDebug() << "Grid width must be >=1.";
         throw(std::invalid_argument("bad input"));
        }
        //qDebug() << "grid.width:" << m_parameters.gridWidth;
        break;
       }

       case e_gridHeight: {
        m_parameters.gridHeight = list.at(1).toInt();
        if (m_parameters.gridHeight < 1) {
         m_valid = false;
         qDebug() << "Grid height must be >=1.";
         throw(std::invalid_argument("bad input"));
        }
        //qDebug() << "grid.height:" << m_parameters.gridHeight;
        break;
       }

       case e_gridDepth: {
        m_parameters.gridDepth = list.at(1).toInt();
        if (m_parameters.gridDepth < 1) {
         m_valid = false;
         qDebug() << "Grid depth must be >=1.";
         throw(std::invalid_argument("bad input"));
        }
        //qDebug() << "grid.depth:" << m_parameters.gridDepth;
        break;
       }

       case e_zDefect:  {
        m_parameters.zDefect=list.at(1).toInt();
        if (m_parameters.zDefect < -1 || m_parameters.zDefect > 1) {
         m_valid = false;
         qDebug() << "Charge on defect must be +/- 1e";
         throw(std::invalid_argument("bad input"));
        }
        //qDebug() << "z.defect: " << m_parameters.zDefect;
        break;
       }
			  
       case e_zTrap: {
        m_parameters.zTrap=list.at(1).toInt();
        if (m_parameters.zTrap < -1 || m_parameters.zTrap>1){
         m_valid = false;
         qDebug() << "Charge on trap must be +/- 1e";
         throw(std::invalid_argument("bad input"));
        }
        //qDebug() << "zTrap: " << m_parameters.zTrap;
        break;
       }

       case e_gridCharge: {
        QString gridCharge = list.at(1).trimmed().toLower();
        if (gridCharge == "true") {
         m_parameters.gridCharge = true;
        }
        else if (gridCharge == "false") {
          m_parameters.gridCharge = false;
        }
        else {
         m_valid = false;
         qDebug() << "Charging the grid is either true or false:" << gridCharge;
         throw(std::invalid_argument("bad input"));
        }
        //qDebug() << "grid.charge:" << m_parameters.gridCharge;
        break;
       }

       case e_potentialForm: {
        QString potentialForm = list.at(1).trimmed().toLower();
        if (potentialForm == "linear") {
         m_parameters.potentialForm = SimulationParameters::o_linearpotential;
        }
        else {
         m_valid = false;
         qDebug() << "potential.form must be linear:" << m_parameters.potentialForm;
         throw(std::invalid_argument("bad input"));
        }
        //qDebug() << "potential.form" << m_parameters.potentialForm;
        break;
       }

       case e_iterationsWarmup: {
        m_parameters.iterationsWarmup = list.at(1).toInt();
        if (m_parameters.iterationsWarmup < 0) {
         m_valid = false;
         qDebug() << "Warmup iterations must be >=0.";
         throw(std::invalid_argument("bad input"));
        }
        //qDebug() << "iterations.warmup:" << m_parameters.iterationsWarmup;
        break;
       }

       case e_iterationsReal: {
        m_parameters.iterationsReal = list.at(1).toInt();
        if (m_parameters.iterationsReal < 1) {
         m_valid = false;
         qDebug() << "Real iterations must be >=1.";
         throw(std::invalid_argument("bad input"));
        }
        //qDebug() << "iterations.real:" << m_parameters.iterationsReal;
        break;
       }

       case e_iterationsPrint: {
        m_parameters.iterationsPrint = list.at(1).toInt();
        if (m_parameters.iterationsPrint < 0) {
         m_valid = false;
         qDebug() << "Print iterations must be >=0.";
         throw(std::invalid_argument("bad input"));
        }
        //qDebug() << "iterations.print:" << m_parameters.iterationsPrint;
        break;
       }

       case e_iterationsTraj: {
        m_parameters.iterationsTraj = list.at(1).toInt();
        if (m_parameters.iterationsTraj < 0) {
         m_valid = false;
         qDebug() << "Traj iterations must be >=0.";
         throw(std::invalid_argument("bad input"));
        }
        //qDebug() << "iterations.traj:" << m_parameters.iterationsTraj;
        break;
       }

       case e_coulombInteraction: {
        QString interaction = list.at(1).trimmed().toLower();
        if (interaction == "true") {
         m_parameters.coulomb = true;
        }
        else if (interaction == "false") {
         m_parameters.coulomb = false;
        }
        else {
         m_valid = false;
         qDebug() << "Coulomb interactions are either true or false:" << interaction;
         throw(std::invalid_argument("bad input"));
        }
         //qDebug() << "interaction.coulomb:" << m_parameters.coulomb;
         break;
        }

        case e_defectsCharged:  {
         QString interaction = list.at(1).trimmed().toLower();
         if (interaction == "true") {
          m_parameters.defectsCharged = true;
         }
         else if (interaction == "false") {
          m_parameters.defectsCharged = false;
         }
         else {
          m_valid = false;
          qDebug() << "Charged defects are either true or false: ";
          throw(std::invalid_argument("bad input"));
         }
         //qDebug() << "charged.defects: " << m_parameters.defectsCharged;
         break;
        }			

        case e_trapsCharged:  {
         QString interaction = list.at(1).trimmed().toLower();
         if (interaction == "true") {
          m_parameters.trapsCharged = true;
         }
         else if (interaction == "false") {
          m_parameters.trapsCharged = false;
         }
         else {
          m_valid = false;
          qDebug() << "Charged traps are either true or false: ";
          throw(std::invalid_argument("bad input"));
         }
         //qDebug() << "charged.traps: " << m_parameters.trapsCharged;
         break;
        }
			
        case e_iterationsXYZ:  {
         QString interaction = list.at(1).trimmed().toLower();
         if (interaction == "true") {
          m_parameters.iterationsXYZ = true;
         }
         else if (interaction == "false") {
          m_parameters.iterationsXYZ = false;
         }
         else {
          m_valid = false;
          qDebug() << "iterations.xyz is either true or false: ";
          throw(std::invalid_argument("bad input"));
         }
         //qDebug() << "iterations.xyz: " << m_parameters.iterationsXYZ;
         break;
        }

        case e_potentialPoint:  {
         QStringList q = list.at(1).trimmed().toLower().remove("(").remove(")").split(",");
         if ( q.length() != 4 ) {
          m_valid = false;
          qDebug() << "Invalid potential.point specified: " << q;
          throw(std::invalid_argument("bad input"));
         }
         bool ok = true;
         double x = q[0].toDouble(&ok);
         if ( !ok ) 
         {
          qDebug() << "0 Invalid potential.point specified: " << q << ok;
          throw(std::invalid_argument("bad input"));
         }
         double y = q[1].toDouble(&ok);
         if ( !ok ) 
         {
          qDebug() << "1 Invalid potential.point specified: " << q << ok;
          throw(std::invalid_argument("bad input"));
         }
         double z = q[2].toDouble(&ok);
         if ( !ok ) 
         {
          qDebug() << "2 Invalid potential.point specified: " << q << ok;
          throw(std::invalid_argument("bad input"));
         }
         double V = q[3].toDouble(&ok);
         if ( !ok ) 
         {
          qDebug() << "3 Invalid potential.point specified: " << q << ok;
          throw(std::invalid_argument("bad input"));
         }
         m_parameters.potentialPoints.push_back(PotentialPoint(x,y,z,V));
         //qDebug() << "potential.point: " << q;
         break;
        }

        case e_potentialSTDEV: {
         m_parameters.gaussianSTDEV = list.at(1).toDouble();
         if (m_parameters.gaussianSTDEV <= 0.00) {
          m_valid = false;
          qDebug() << "Negative or zero standard deviation specified for random noise";
          throw(std::invalid_argument("bad input"));
         }
         //qDebug() << "potential.stdev: " << m_parameters.gaussianSTDEV;
         break;
        }

        case e_potentialAVERG: {
         m_parameters.gaussianAVERG = list.at(1).toDouble();
         //qDebug() << "potential.averg: " << m_parameters.gaussianAVERG;
         break;
        }

        case e_potentialNoise:  {
         QString choice = list.at(1).trimmed().toLower();
         if (choice == "true") {
          m_parameters.gaussianNoise = true;
         }
         else if (choice == "false") {
          m_parameters.gaussianNoise = false;
         }
         else {
          m_valid = false;
          qDebug() << "potential.noise is either true or false: ";
          throw(std::invalid_argument("bad input"));
         }
         //qDebug() << "potential.noise: " << m_parameters.gaussianNoise;
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
    s_variables["defect.percentage"] = e_defectPercentage;
    s_variables["trap.percentage"] = e_trapPercentage;
    s_variables["charge.percentage"] = e_chargePercentage;
    s_variables["temperature.kelvin"] = e_temperatureKelvin;
    s_variables["delta.epsilon"] = e_deltaEpsilon;
    s_variables["variable.working"] = e_variableWorking;
    s_variables["variable.start"] = e_variableStart;
    s_variables["variable.final"] = e_variableFinal;
    s_variables["variable.steps"] = e_variableSteps;
    s_variables["grid.width"] = e_gridWidth;
    s_variables["grid.height"] = e_gridHeight;
    s_variables["grid.depth"] = e_gridDepth;
    s_variables["z.defect"] = e_zDefect;
    s_variables["z.trap"] = e_zTrap;
    s_variables["grid.charge"] = e_gridCharge;
    s_variables["iterations.warmup"] = e_iterationsWarmup;
    s_variables["iterations.real"] = e_iterationsReal;
    s_variables["iterations.print"] = e_iterationsPrint;
    s_variables["iterations.traj"] = e_iterationsTraj;
    s_variables["iterations.xyz"] = e_iterationsXYZ;
    s_variables["interaction.coulomb"] = e_coulombInteraction;
    s_variables["charged.defects"] = e_defectsCharged;
    s_variables["charged.traps"] = e_trapsCharged;
    s_variables["potential.form"] = e_potentialForm;
    s_variables["potential.point"] = e_potentialPoint;
    s_variables["potential.noise"] = e_potentialNoise;
    s_variables["potential.stdev"] = e_potentialSTDEV;
    s_variables["potential.averg"] = e_potentialAVERG;
  }

}
