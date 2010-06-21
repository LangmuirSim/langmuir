#ifndef _POTENTIAL_H
#define _POTENTIAL_H

#include<iostream>
#include<cmath>
#include<stdexcept>
#include"potentialpoint.h"
#include<QTextStream>

namespace Langmuir
{
  /**
    *  @class Potential.
    *  @brief The potential in a system produced by the enviroment.
    *
    *  Pure virtual base class to inherited by all externalfield classes.
    *  @date 06/15/2010
    */
  class Potential
  {

  public:

    /**
     * @brief virtual Destructor.
     */
    virtual ~Potential()
    {
    }

    /**
     * @brief A method to insert a point of defined potential
     *
     * We sometimes want base pointers to add points of defined potential.
     * Only the expert derived classes know how to add them correctly.
     * @param point a point of defined potential
     */
    virtual void addPoint( PotentialPoint point ) = 0;

    /**
     * @brief A method to insert a point of defined potential for the source
     *
     * We sometimes want base pointers to add points of defined potential.
     * Only the expert derived classes know how to add them correctly.
     * @param point a point of defined potential
     */
    virtual void addSourcePoint( PotentialPoint point ) = 0;

    /**
     * @brief A method to insert a point of defined potential for the drain
     *
     * We sometimes want base pointers to add points of defined potential.
     * Only the expert derived classes know how to add them correctly.
     * @param point a point of defined potential
     */
    virtual void addDrainPoint( PotentialPoint point ) = 0;

    /**
     * @brief calculate the potential
     *
     * Use internal information about the potential to determine the value at this point.
     * @param col index of column
     * @param row index of row
     * @param lay index of layer
     * @return potential value of potential at this location
     */  
    virtual double calculate( double col, double row, double lay ) = 0;

    /**
      * @brief stream operator overload to print potential
      * 
      * The potential is printed in a way so that it may be plotted.
      * The virtual friend function idiom is being used here.
      * So do not alter this function, instead alter the virtual function print.
      * @param os reference to stream object
      * @param potential reference to potential object
      * @return os reference to ostream object originally passed
      */
    friend std::ostream& operator<<(std::ostream& os, Potential& potential)
    {
     return potential.print(os);
    }

    /**
      * @brief print a slice of potential for plotting
      * 
      * The potential is printed in a way so that it may be plotted.
      * @param stream write to this stream
      * @param xstart starting value of x
      * @param xstop starting value of x
      * @param ystart starting value of y
      * @param ystop starting value of y
      * @param zvalue value of z
      * @param xdelta delta value for x
      * @param ydelta delta value for y
      */
    virtual void plot( QTextStream& stream, double xstart = -10.0, double xstop = 10.0, double ystart = -10.0, double ystop = 10.0, double zvalue = 0.0, double xdelta = 1.0, double ydelta = 1.0 ) = 0; 

  protected:

    /**
      * @brief implementation of operator<< overload
      * @param os reference to stream object
      * @return os reference to ostream object originally passed
      */
    virtual std::ostream& print(std::ostream& os) = 0;

  };

}
#endif
