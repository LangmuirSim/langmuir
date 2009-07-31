/**
 * InputParser parses an input file line by line, and sets up a simulation run
 * based upon the text input. This class operates on an instantiated Simulation
 * object, as well as provinding the variable that should be changed.
 */

#ifndef INPUTPARSER_H
#define INPUTPARSER_H

#include <QMap>

#include <vector>

class QString;
class QIODevice;

namespace Langmuir {

  /**
   * @struct All pertinent simulation parameters are stored in here. This can
   * easily be serialized and sent over MPI etc.
   */
  struct SimulationParameters{
    SimulationParameters() : voltageSource(0.0), voltageDrain(0.0),
      trapPercentage(0.0), chargePercentage(0.01),
      gridWidth(10), gridHeight(10),
      iterationsWarmup(1000), iterationsReal(5000), iterationsPrint(500),
      coulomb(false) { }

    double voltageSource, voltageDrain, trapPercentage, chargePercentage;
    int gridWidth, gridHeight;
    int iterationsWarmup, iterationsReal, iterationsPrint;
    bool coulomb;
  };

  class InputParser
  {
  public:
    /**
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
      e_trapPercentage,     // percentage of traps in the grid
      e_chargePercentage,   // percentage of charges in the grid - sets as target
      e_variableWorking,    // the working variable that is being changed
      e_variableStart,      // the start of the variables range
      e_variableFinal,      // final value of the variable range
      e_variableSteps,      // the number of steps to take from start to final
      e_gridWidth,          // the width of the grid
      e_gridHeight,         // the height of the grid
      e_iterationsWarmup,   // the number of warm up iterations to perform
      e_iterationsReal,     // the number of iterations for the real run
      e_iterationsPrint,    // number of iterations before printing state
      e_coulombInteraction, // should Coulomb interaction be used
      e_end
    };

    /**
     * @return The variable that is being modified in the simulation.
     */
    VariableType varyType() const { return m_varyType; }

    /**
     * @return The start value of the variable that is being looped over.
     */
    double start() const { return m_start; }

    /**
     * @return The final value of the variable that is being looped over.
     */
    double final() const { return m_final; }

    /**
     * @return The number of steps to take moving between the start and finish
     * values.
     */
    int steps() const { return m_steps; }

    /**
     * @return The size of each step.
     */
    double stepSize() const { return (m_final - m_start) / double(m_steps); }

    double voltageSource() const { return m_parameters.voltageSource; }

    double voltageDrain() const { return m_parameters.voltageDrain; }

    double trapPercentage() const { return m_parameters.trapPercentage; }

    int gridWidth() const { return m_parameters.gridWidth; }

    int gridHeight() const { return m_parameters.gridHeight; }

    /**
     * Get the simulation parameters for the supplied step.
     * @param par The SimulationParameters @struct that will be set up.
     * @param step The step
     */
    bool simulationParameters(SimulationParameters *par, int step = 0);

    /**
     * @return A QString with the name of the type that is being varied.
     */
    QString workingVariable() const;

  private:
    VariableType m_varyType; // The variable that is being varied
    double m_start;
    double m_final;
    int m_steps;
    bool m_valid;

    SimulationParameters m_parameters; // Simulation parameters

    /**
     * Map of the input keys to the different simulation parameters
     */
    static QMap<QString, VariableType> s_variables;

    /**
     * Process a line of input.
     */
    void processLine(QIODevice *file);

    /**
     * Initialize the static map of variables if necessary
     */
    void initializeVariables();
  };

}

#endif // INPUTPARSER_H
