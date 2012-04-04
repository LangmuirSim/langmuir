#ifndef TOLERANCE_H
#define TOLERANCE_H

#include <QObject>

namespace Langmuir
{

class World;
class FluxAgent;

//! A class to check if the simulation is converging
class Tolerance : public QObject
{
    Q_OBJECT
public:
    //! Create the tolerance checking object
    /*!
      \param reference to the world object
      \param a flux agent to track
      \param the criterion for convergence
      \param parent the parent of this QObject
      */
    explicit Tolerance(World &world, FluxAgent *flux, double criteria = 0.01, QObject *parent = 0);

    //! check if the convergence criteria has been met
    void check();

    //! true if the convergence criteria has been met the appropriate number of times
    bool converged();

private:
    //! current flux success/step ratio
    double m_current;

    //! previous flux success/step ratio
    double m_previous;

    //! the maximum percent difference between previous and current for the criteria to be met
    double m_criteria;

    //! the number of consecutive times the flux agent has met the convergence criteria
    int m_converged;

    //! the flux agent being tracked
    FluxAgent *m_flux;

    //! reference to the world object
    World &m_world;

    //! make sure the criteria is a valid percent
    void checkCritera();

    //! set the previous success/step ratio to be whatever flux currently reports
    void setPrevious();

    //! set the current success/step ratio to whatever flux currently reports
    void setCurrent();

    //! calculate the percent change between previous and current flux success/step ratios
    double percentChange();
};

}
#endif // TOLERANCE_H
