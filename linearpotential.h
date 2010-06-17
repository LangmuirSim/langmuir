#ifndef _LINEARPOTENTIAL_H
#define _LINEARPOTENTIAL_H

#include"potential.h"
#include<vector>
#include<algorithm>
#include<stdexcept>

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
    double operator()( int col, int row, int lay );

    /**
      * @brief add a point of defined potential along x
      * 
      * This point is added to a list of points of defined potential along the x axis.
      * The value of the potential is then stored.
      * The points are kept in order of smallest to largest.
      * If more than two points are known along this axis, 
      * the slops and intercepts are calculated between the points.
      */
    void addXPoint( int col, double potential );

    /**
      * @brief add a point of defined potential along y
      * 
      * This point is added to a list of points of defined potential along the y axis.
      * The value of the potential is then stored.
      * The points are kept in order of smallest to largest.
      * If more than two points are known along this axis, 
      * the slops and intercepts are calculated between the points.
      */
    void addYPoint( int row, double potential );

    /**
      * @brief add a point of defined potential along z
      * 
      * This point is added to a list of points of defined potential along the z axis.
      * The value of the potential is then stored.
      * The points are kept in order of smallest to largest.
      * If more than two points are known along this axis, 
      * the slops and intercepts are calculated between the points.
      */
    void addZPoint( int lay, double potential );

  protected:

    /**
      * @brief implementation of operator<< overload
      * @param os reference to stream object
      * @return os reference to ostream object originally passed
      */
    virtual std::ostream& print(std::ostream& os);
    
    /**
      * @brief points along x with defined potential
      *
      * The point is (x,y,z,V) = (x,0,0,V)
      */
    std::vector<PotentialPoint> xx;

    /**
      * @brief points along y with defined potential
      *
      * The point is (x,y,z,V) = (0,y,0,V)
      */
    std::vector<PotentialPoint> yy;

    /**
      * @brief points along z with defined potential
      *
      * The point is (x,y,z,V) = (0,0,z,V)
      */
    std::vector<PotentialPoint> zz;
   
    /**
      * @brief slopes along x between points of defined potential 
      * @warning with N points of potential defined, there are N-1 slopes if N >= 2.
      */
    std::vector<double> mx;

    /**
      * @brief slopes along y between points of defined potential 
      * @warning with N points of potential defined, there are N-1 slopes if N >= 2.
      */
    std::vector<double> my;

    /**
      * @brief slopes along z between points of defined potential 
      * @warning with N points of potential defined, there are N-1 slopes if N >= 2.
      */
    std::vector<double> mz;
 
    /**
      * @brief intercepts along x between points of defined potential 
      * @warning with N points of potential defined, there are N-1 intercepts if N >= 2.
      */
    std::vector<double> bx;

    /**
      * @brief intercepts along y between points of defined potential 
      * @warning with N points of potential defined, there are N-1 intercepts if N >= 2.
      */
    std::vector<double> by;

    /**
      * @brief intercepts along z between points of defined potential 
      * @warning with N points of potential defined, there are N-1 intercepts if N >= 2.
      */
    std::vector<double> bz;

    /**
      * @brief determine which region along x a point is located in
      *
      * Given the non uniform set of points of defined potential along x, 
      * determine between which two of these points a given value of x lies.
      * @param x a given value of x
      * @return i index of slope and intercept array corresponding to the region bounded by the two points containing x
      */
    unsigned int regionX( int x );

    /**
      * @brief determine which region along y a point is located in
      *
      * Given the non uniform set of points of defined potential along y, 
      * determine between which two of these points a given value of y lies.
      * @param y a given value of y
      * @return i index of slope and intercept array corresponding to the region bounded by the two points containing y
      */
    unsigned int regionY( int y );

    /**
      * @brief determine which region along z a point is located in
      *
      * Given the non uniform set of points of defined potential along z, 
      * determine between which two of these points a given value of z lies.
      * @param z a given value of z
      * @return i index of slope and intercept array corresponding to the region bounded by the two points containing z
      */
    unsigned int regionZ( int z );

  };
}
#endif
