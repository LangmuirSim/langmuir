#ifndef INPUTPARSER_H
#define INPUTPARSER_H

#include <QMap>
#include <vector>
#include "potential.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>
class QString;
class QIODevice;

namespace Langmuir {

  /**
   * @struct All pertinent simulation parameters are stored in here. This can
   * easily be serialized and sent over MPI etc.
   */
  struct SimulationParameters {

    /**
      * @brief operator overload to print parameters
      * @return reference to QTextStream
      */
    friend  QTextStream& operator<<(  QTextStream& qt, SimulationParameters& par );

    enum Option {

     o_linearpotential

    };

    double      voltageSource; 
    double       voltageDrain;
    double   defectPercentage;
    double     trapPercentage;
    double   chargePercentage;
    double  temperatureKelvin;
    double       deltaEpsilon;
    double      gaussianSTDEV;
    double      gaussianAVERG;

    int             gridWidth;
    int            gridHeight;
    int             gridDepth;
    int               zDefect;
    int                 zTrap;
    int      iterationsWarmup; 
    int        iterationsReal;
    int       iterationsPrint;
    int        iterationsTraj; 

    bool              coulomb;
    bool       defectsCharged;
    bool         trapsCharged;
    bool           gridCharge;
    bool        iterationsXYZ;
    bool        gaussianNoise;
	bool          trapsHetero;

    Option      potentialForm;
    std::vector<PotentialPoint> potentialPoints;

    SimulationParameters()
    {
     voltageSource         =               0.00;
     voltageDrain          =               1.00;
     defectPercentage      =               0.00;
     trapPercentage        =               0.00;
     chargePercentage      =               0.01;
     temperatureKelvin     =             300.00;
     deltaEpsilon          =               0.00;
     gridWidth             =               1024;
     gridHeight            =                256;
     gridDepth             =                  1;
     zDefect               =                  0;
     zTrap                 =                  0;
     iterationsWarmup      =               1000;
     iterationsReal        =              10000;
     iterationsPrint       =               1000;
     iterationsTraj        =               1000;
     coulomb               =              false;
     defectsCharged        =              false;
     trapsCharged          =              false;
     gridCharge            =              false;
     iterationsXYZ         =              false;
     potentialForm         =  o_linearpotential;
     gaussianNoise         =              false;
	 gaussianSTDEV         =               0.05;
     gaussianAVERG         =               0.00;
     trapsHetero           =              false;
	}
  };

  /**
    *  @class InputParser
    *  @brief Input reading class.
    *
    *  InputParser parses an input file line by line, and sets up a simulation run
    *  based upon the text input. This class operates on an instantiated Simulation
    *  object, as well as provinding the variable that should be changed.
    *  @date 06/07/2010
    */
  class InputParser
  {
  public:
    /**
     * @brief Constructor.
     *
     * Constructor. This is used to set up the Simulation instance, and saves
     * the parameters that should be varied in the simulation instance. These
     * parameters may be varied in serial fashion, or (TBC) distributed over the
     * cluster using MPI.
     *
     * @param fileName The name of the file to parse for simulation parameters.
     * @param simulation The simulation object that should be set up with the
     * parameters in the input file.
     */
    InputParser(const QString& fileName);

    /**
     * @enum The supported variables that can be varied - only one may be varied
     * in a simulation, although this could be extended to support more if the
     * need arises.
     */
    enum VariableType {
      e_undefined,
      e_voltageSource,      // voltage of the source electrode
      e_voltageDrain,       // voltage of the drain electrode
      e_defectPercentage,   // percentage of defects in the grid
      e_trapPercentage,     // percentage of traps in the grid
      e_chargePercentage,   // percentage of charges in the grid - sets as target
      e_temperatureKelvin,  // the absolute temperature
      e_deltaEpsilon,       // site energy difference for traps
      e_variableWorking,    // the working variable that is being changed
      e_variableStart,      // the start of the variables range
      e_variableFinal,      // final value of the variable range
      e_variableSteps,      // the number of steps to take from start to final
      e_gridWidth,          // the width of the grid
      e_gridHeight,         // the height of the grid
      e_gridDepth,          // the depth of the grid
      e_zDefect,            // charge on the defects (times e)
      e_zTrap,              // charge on traps (times e)
      e_gridCharge,         // seed the grid with charges
      e_iterationsWarmup,   // the number of warm up iterations to perform
      e_iterationsReal,     // the number of iterations for the real run
      e_iterationsPrint,    // number of iterations before printing state
      e_iterationsTraj,     // number of iteratiobs before printing trajectory
      e_coulombInteraction, // should Coulomb interaction be used
      e_defectsCharged,     // are the defects charged?
      e_trapsCharged,       // are the traps charged?
      e_iterationsXYZ,      // should trajectory files be written
      e_potentialForm,      // V = Vx + Vy + Vz (only option currently)
      e_potentialPoint,     // A point of defined potential (x,y,z,V)
      e_potentialNoise,     // Should random noise be added to energy levels
      e_potentialSTDEV,     // What is the standard deviation of the random noise
      e_potentialAVERG,     // What is the average of the random noise
	  e_trapsHetero,        // Distrubute traps heterogeneously?  
      e_end
    };

    /**
      * @brief operator overload to print input parser
      * @return reference to QTextStream
      */
    friend  QTextStream& operator<<(  QTextStream& qt, InputParser& inp );

    /**
     * @brief Working variable.
     * @return The variable that is being modified in the simulation.
     */
    VariableType varyType() const { return m_varyType; }

    /**
     * @brief Working varibale start.
     * @return The start value of the variable that is being looped over.
     */
    double start() const { return m_start; }

    /**
     * @brief Working variable stop.
     * @return The final value of the variable that is being looped over.
     */
    double final() const { return m_final; }

    /**
     * @brief Working variable steps.
     * @return The number of steps to take moving between the start and finish
     * values.
     */
    int steps() const { return m_steps; }

    /**
     * @brief Working variable step size.
     * @return The size of each step.
     */
    double stepSize() const { return (m_final - m_start) / double(m_steps); }
    double voltageSource() const { return m_parameters.voltageSource; }
    double voltageDrain() const { return m_parameters.voltageDrain; }
    double defectPercentage() const { return m_parameters.defectPercentage; }
    double trapPercentage() const { return m_parameters.trapPercentage; }
    double temperatureKelvin() const { return m_parameters.temperatureKelvin;}
    double deltaEpsilon() const { return m_parameters.deltaEpsilon;}
       int gridWidth() const { return m_parameters.gridWidth; }
       int gridHeight() const { return m_parameters.gridHeight; }
       int gridDepth() const { return m_parameters.gridDepth; }
       int zDefect() const { return m_parameters.zDefect; }

    /**
     * Get the simulation parameters for the supplied step.
     * @param par The SimulationParameters @struct that will be set up.
     * @param step The step
     */
    bool simulationParameters(SimulationParameters *par, int step = 0);

    /**
     * @brief Working variable name.
     * @return A QString with the name of the type that is being varied.
     */
    QString workingVariable() const;

  private:
    /** 
     * @brief Working variable type.
     * 
     * The variable that is changing.
     */
    VariableType m_varyType;

    /** 
     * @brief Working variable start.
     * 
     * The starting value of the variable that is changing.
     */
    double m_start;

    /** 
     * @brief Working variable stop.
     * 
     * The ending value of the variable that is changing.
     */
    double m_final;

    /** 
     * @brief Working variable steps.
     * 
     * The number of values between start and stop for the variable that is changing.
     */
    int m_steps;

    /** 
     * @brief Valid.
     * 
     * Unknown.
     */
    bool m_valid;

    /** 
     * @brief Working variable current step.
     * 
     * The current step of the working variable.
     */
    int m_step;

    /** 
     * @brief Working variable current value.
     * 
     * The current value of the working variable.
     */
    double m_value;

    /** 
     * @brief The values of parameters.
     * 
     * Struct of variable parameters.
     */
    SimulationParameters m_parameters;

    /**
     * @brief string to variable type map
     *
     * Map of the input keys to the different simulation parameters
     */
    static QMap<QString, VariableType> s_variables;

    /**
     * @brief Line read.
     *
     * Process a line of input.
     */
    void processLine(QIODevice *file);

    /**
     * @brief set up map.
     *
     * Initialize the static map of variables if necessary
     */
    void initializeVariables();
  };

}
#endif // INPUTPARSER_H
