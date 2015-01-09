#ifndef FLUXAGENT_H
#define FLUXAGENT_H

#include "agent.h"
#include "cubicgrid.h"

namespace LangmuirCore
{

struct SimulationParameters;

/**
 * @brief A class to change the number of carriers in the system
 *
 * A flux agent can inject carriers (Agent::Source) or accept carriers (Agent::Drain)
 */
class FluxAgent : public Agent
{
public:    
    /**
     * @brief Create the flux agent
     * @param type either a Agent::Source or Agent::Drain
     * @param world reference to world object
     * @param grid reference to grid
     * @param parent parent QObject
     */
    FluxAgent(Agent::Type type, World &world, Grid& grid, QObject *parent = 0);

    /**
     * @brief unregisters FluxAgent from the grid
     */
    ~FluxAgent();

    /**
     * @brief set the FluxAgent's potential
     * @param potential the value of the potential
     */
    void setPotential(double potential);

    /**
     * @brief get the FluxAgent's potential
     */
    double potential() const;

    /**
     * @brief set the FluxAgent's rate
     * @param rate the value of the rate
     */
    void setRate(double rate);

    /**
     * @brief set the FluxAgent's rate
     * @param rate the value of the rate
     * @param dflt the default value to set the rate to
     *
     * If rate is negative, uses the default rate instead
     */
    void setRateSmartly(double rate, double dflt);

    /**
     * @brief get the FluxAgent's rate
     */
    double rate() const;

    /**
     * @brief set the FluxAgent's attempt counter
     * @param value the value of the attempt counter
     * @warning also calls storeLast()
     */
    void setAttempts(unsigned long int value);

    /**
     * @brief get the FluxAgent's attempt counter
     */
    unsigned long int attempts() const;

    /**
     * @brief set the FluxAgent's success counter
     * @param value the value of the counter
     * @warning also calls storeLast()
     */
    void setSuccesses(unsigned long int value);

    /**
     * @brief get the FluxAgent's success counter
     */
    unsigned long int successes() const;

    /**
     * @brief set the value of last to the value of successes, and store the current step
     */
    void storeLast();

    /**
     * @brief get the number of successes since storeLast() was called
     */
    unsigned long int successesSinceLast() const;

    /**
     * @brief get the number of attempts since storeLast() was called
     */
    unsigned long int attemptsSinceLast() const;

    /**
     * @brief get the number of steps since storeLast() was called
     */
    unsigned long int stepsSinceLast() const;

    /**
     * @brief calculate and return the current probabilty of success
     *
     * This is the number of successes divided by the number of attempts (x100).
     * Ideally, this number should approach probability() as the simulation
     * progresses, if shouldTransport() uses the simple constant probability
     * method.
     */
    double successProbability()const;

    /**
     * @brief calculate and return the current rate of success
     *
     * This is the number of successes divided by the number of simulation steps.
     * The current is related to the rate.
     */
    double successRate()const;

    /**
     * @brief calculate and return the probabilty of success since storeLast() was called
     *
     * This is the number of successesSinceLast() divided by the number of attemptsSinceLast() (x100).
     */
    double successProbabilitySinceLast()const;

    /**
     * @brief calculate and return the rate of success since storeLast() was called
     *
     * This is the number of successesSinceLast() divided by the number of stepsSinceLast().
     * The current is related to the rate.
     */
    double successRateSinceLast()const;

    /**
     * @brief set the attempt and success counters to zero
     */
    void resetCounters();

    /**
     * @brief get the Grid:CubeFace this FluxAgent is assigned to
     */
    Grid::CubeFace face()const;

    /**
     * @brief get the Grid this FluxAgent belongs to
     */
    Grid& grid()const;

protected:
    /**
     * @brief assign the FluxAgent to a specific site in the grid
     * @param site the site in the grid
     */
    void initializeSite(int site);

    /**
     * @brief assign the FluxAgent to a specific Grid::CubeFace
     * @param cubeFace the face of a cubic grid; for example Grid::Left
     *
     * When assigning to a specific Grid::CubeFace, the FluxAgent is considered
     * to be a special agent, and thus resides in the sites reserved by the
     * grid for special agents.
     */
    void initializeSite(Grid::CubeFace cubeFace);

    /**
     * @brief decide if the FluxAgent should transport a carrier to/from a given site
     * @param site the site involved
     * @return true if the FluxAgent should transport to/from the site
     *
     * The default behavoir is for the FluxAgent to use a simple constant probabilty to make this
     * decision.  However, classes derived from FluxAgent can reimplement this function.  For
     * example, one might want to use a Metropolis criterion to make this decision.
     */
    virtual bool shouldTransport(int site);

    /**
     * @brief The energy change associated with moving a carrier from the FluxAgent to a site
     * @param site the site involved
     * @return the energy change
     */
    virtual double energyChange(int site);

    /**
     * @brief convert the Grid::CubeFace to a single letter
     *
     * For example, Grid::Left would return \b L.  This is used in the output file titles.
     */
    QString faceToLetter();

    /**
     * @brief the number of times the FluxAgent has tried to transport.
     */
    unsigned long int m_attempts;

    /**
     * @brief the number of times the FluxAgent was successful in transporting.
     */
    unsigned long int m_successes;

    /**
     * @brief storage to note the number of successes at some step
     */
    unsigned long int m_lastSuccesses;

    /**
     * @brief storage to note the number of successes at some step
     */
    unsigned long int m_lastAttempts;

    /**
     * @brief the step at which last was noted
     */
    unsigned long int m_lastStep;

    /**
     * @brief the constant probability used in the default behavoir of shouldTransport().
     */
    double m_probability;

    /**
     * @brief the potential that is (possibly) used when calculating an energy change
     *
     * The energy change can be used in the shouldTransport() function.
     */
    double m_potential;

    /**
     * @brief the grid this FluxAgent resides in
     */
    Grid &m_grid;

    /**
     * @brief the face of the grid this FluxAgent occupies
     *
     * It may be Grid::NoFace is the FluxAgent occupies an actual site.
     */
    Grid::CubeFace m_face;
};

}
#endif
