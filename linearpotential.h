#ifndef _LINEARPOTENTIAL_H
#define _LINEARPOTENTIAL_H

#include"potential.h"
#include<algorithm>

namespace Langmuir
{
  /**
    *  @class LinearPotential.
    *  @brief The potential in a system produced by the enviroment.
    *
    *  A potential that can be written as V(x,y,z) = V1(x) + V2(y) + V3(z).
    *  The functions V1, V2, and V3 are independent and linear in x, y, and z.
    *  The electric field -delV = <V1'(x),V2'(y),V3'(z)> is therefore constant.
    *  The potential is linearly interpolated between defined points on V1, V2, and V3.
    *  @date 06/15/2010
    */
  class LinearPotential : public Potential

  {

  public:

    /**
     * @brief Constructor
     */
    LinearPotential();

    /**
     * @brief Constructor
     *
     * Takes a list of potential points and sets up the potential calculator.
     * Checks to see that points are defined only along an axis, and not arbitrarily in space.
     * @param PotentialPoints a list of potential points of the form (x,0,0,V)... (0,y,0,V)... (0,0,z,V)...
     */
    LinearPotential( QVector<PotentialPoint>& PotentialPoints );
  
    /**
     * @brief Destructor.
     */
    ~LinearPotential();

    /**
     * @brief calculate the potential
     *
     * Use internal information about the potential to determine the value at this point.
     * @param col index of column
     * @param row index of row
     * @param lay index of layer
     * @return potential value of potential at this location
     */ 
    double calculate( double col, double row, double lay );

    /**
     * @brief A method to insert a point of defined potential
     *
     * We sometimes want base pointers to add points of defined potential.
     * Only the expert derived classes know how to add them correctly.
     * @param point a point of defined potential
     */
    void addPoint( PotentialPoint point );

    /**
     * @brief A method to insert a point of defined potential for the source
     *
     * We sometimes want base pointers to add points of defined potential.
     * Only the expert derived classes know how to add them correctly.
     * @param point a point of defined potential
     */
    void addSourcePoint( PotentialPoint point );

    /**
     * @brief A method to insert a point of defined potential for the drain
     *
     * We sometimes want base pointers to add points of defined potential.
     * Only the expert derived classes know how to add them correctly.
     * @param point a point of defined potential
     */
    void addDrainPoint( PotentialPoint point );


    /**
      * @brief add a point of defined potential along x
      * 
      * This point is added to a list of points of defined potential along the x axis.
      * The value of the potential is then stored.
      * The points are kept in order of smallest to largest.
      * If more than two points are known along this axis, 
      * the slops and intercepts are calculated between the points.
      * If points are not added in order, all slopes and intercepts must be recalculated.
      */
    void addXPoint( double col, double potential );

    /**
      * @brief overload of addXPoint
      * 
      */
    void addXPoint( PotentialPoint& p );

    /**
      * @brief add a point of defined potential along y
      * 
      * This point is added to a list of points of defined potential along the y axis.
      * The value of the potential is then stored.
      * The points are kept in order of smallest to largest.
      * If more than two points are known along this axis, 
      * the slops and intercepts are calculated between the points.
      * If points are not added in order, all slopes and intercepts must be recalculated.
      */
    void addYPoint( double row, double potential );

    /**
      * @brief overload of addYPoint
      * 
      */
    void addYPoint( PotentialPoint& p );

    /**
      * @brief add a point of defined potential along z
      * 
      * This point is added to a list of points of defined potential along the z axis.
      * The value of the potential is then stored.
      * The points are kept in order of smallest to largest.
      * If more than two points are known along this axis, 
      * the slops and intercepts are calculated between the points.
      * If points are not added in order, all slopes and intercepts must be recalculated.
      */
    void addZPoint( double lay, double potential );

    /**
      * @brief overload of addZPoint
      * 
      */
    void addZPoint( PotentialPoint& p );

    /**
      * @brief print a slice of potential for plotting
      * 
      * The potential is printed in a way so that it may be plotted.
      * @param xstart starting value of x
      * @param xstop starting value of x
      * @param ystart starting value of y
      * @param ystop starting value of y
      * @param zvalue value of z
      * @param xdelta delta value for x
      * @param ydelta delta value for y
      */
    virtual void plot( QTextStream& stream, double xstart = -10.0, double xstop = 10.0, double ystart = -10.0, double ystop = 10.0, double zvalue = 0.0, double xdelta = 1.0, double ydelta = 1.0 );

  protected:
    
    /**
      * @brief points along x with defined potential
      *
      * The point is (x,y,z,V) = (x,0,0,V)
      */
    QVector<PotentialPoint> xx;

    /**
      * @brief points along y with defined potential
      *
      * The point is (x,y,z,V) = (0,y,0,V)
      */
    QVector<PotentialPoint> yy;

    /**
      * @brief points along z with defined potential
      *
      * The point is (x,y,z,V) = (0,0,z,V)
      */
    QVector<PotentialPoint> zz;
   
    /**
      * @brief slopes along x between points of defined potential 
      * @warning with N points of potential defined, there are N-1 slopes if N >= 2.
      */
    QVector<double> mx;

    /**
      * @brief slopes along y between points of defined potential 
      * @warning with N points of potential defined, there are N-1 slopes if N >= 2.
      */
    QVector<double> my;

    /**
      * @brief slopes along z between points of defined potential 
      * @warning with N points of potential defined, there are N-1 slopes if N >= 2.
      */
    QVector<double> mz;
 
    /**
      * @brief intercepts along x between points of defined potential 
      * @warning with N points of potential defined, there are N-1 intercepts if N >= 2.
      */
    QVector<double> bx;

    /**
      * @brief intercepts along y between points of defined potential 
      * @warning with N points of potential defined, there are N-1 intercepts if N >= 2.
      */
    QVector<double> by;

    /**
      * @brief intercepts along z between points of defined potential 
      * @warning with N points of potential defined, there are N-1 intercepts if N >= 2.
      */
    QVector<double> bz;

    /**
      * @brief determine which region along x a point is located in
      *
      * Given the non uniform set of points of defined potential along x, 
      * determine between which two of these points a given value of x lies.
      * @param x a given value of x
      * @return i index of slope and intercept array corresponding to the region bounded by the two points containing x
      */
    unsigned int regionX( double x );

    /**
      * @brief determine which region along y a point is located in
      *
      * Given the non uniform set of points of defined potential along y, 
      * determine between which two of these points a given value of y lies.
      * @param y a given value of y
      * @return i index of slope and intercept array corresponding to the region bounded by the two points containing y
      */
    unsigned int regionY( double y );

    /**
      * @brief determine which region along z a point is located in
      *
      * Given the non uniform set of points of defined potential along z, 
      * determine between which two of these points a given value of z lies.
      * @param z a given value of z
      * @return i index of slope and intercept array corresponding to the region bounded by the two points containing z
      */
    unsigned int regionZ( double z );

  };
}
#endif
