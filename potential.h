#ifndef _POTENTIAL_H
#define _POTENTIAL_H

#include<iostream>
#include<cmath>

namespace Langmuir
{
  /**
    *  @class EField.
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
     * @brief calculate the potential
     *
     * Use internal information about the potential to determine the value at this point.
     * @param col index of column
     * @param row index of row
     * @param lay index of layer
     * @return potential value of potential at this location
     */  
    virtual double operator()( int col, int row, int lay ) = 0;

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

  protected:

    /**
      * @brief implementation of operator<< overload
      * @param os reference to stream object
      * @return os reference to ostream object originally passed
      */
    virtual std::ostream& print(std::ostream& os) = 0;

  };

  /**
    * @class PotentialPoint
    * @brief container for potential values
    *
    * Assumes integer (non continuous) x, y, and z values.
    * Potential itself is stored as a double.
    * @date 06/15/2010
    */
 class PotentialPoint
 {
 public:

  /**
    * @brief default constructor
    *
    * Initializes a 4 dimensional vector (x,y,z,V) 
    * @param col the integer x value.
    * @param row the integer y value.
    * @param lay the integer z value.
    * @param potential the floating potential value
    */
  PotentialPoint( int col, int row, int lay, double potential )
  {
   this->col = col;
   this->row = row;
   this->lay = lay;
   this->potential = potential;
  }

  /**
    * @brief default destructor
    */
 ~PotentialPoint()
  {
  }

  /**
    * @brief operator << overload 
    *
    * Allows the expression: ostream << PotentialPoint;
    */
  friend std::ostream& operator<<(std::ostream& os, const PotentialPoint& p)
  {
   os << "{ " << p.col << " " << p.row << " " << p.lay << " " << p.potential << " }";
   return os;
  }

  /**
    * @struct comparison functor
    *
    * Used for sorting points with standard library.
    */
  struct less_than_by_x_value
  {
   /**
     * @brief functor operator () overload
     *
     * compare two points by x value and less than operator.
     */
   bool operator()(const PotentialPoint& lhs, const PotentialPoint& rhs) const
   { 
    return lhs.x() < rhs.x(); 
   }
  };

  /**
    * @struct comparison functor
    *
    * Used for sorting points with standard library.
    */
  struct less_than_by_y_value
  {
   /**
     * @brief functor operator () overload
     *
     * compare two points by y value and less than operator.
     */
   bool operator()(const PotentialPoint& lhs, const PotentialPoint& rhs) const
   { 
    return lhs.y() < rhs.y(); 
   }
  };

  /**
    * @struct comparison functor
    *
    * Used for sorting points with standard library.
    */
  struct less_than_by_z_value
  {
   /**
     * @brief functor operator () overload
     *
     * compare two points by z value and less than operator.
     */
   bool operator()(const PotentialPoint& lhs, const PotentialPoint& rhs) const
   { 
    return lhs.z() < rhs.z(); 
   }
  };

 /**
   * @brief access col value
   * @return col value
   */
 inline const int& x() const { return col; }

 /**
   * @brief access row value
   * @return row value
   */
 inline const int& y() const { return row; }

 /**
   * @brief access lay value
   * @return lay value
   */
 inline const int& z() const { return lay; }

 /**
   * @brief access potential value
   * @return potential value
   */
 inline const double& V() const { return potential; }
 
 protected:
  /**
    * @brief value of x coordinate 
    */
  int col;

  /** 
    * @brief value of y coordinate
    */
  int row;

  /**
    * @brief value of z coordinate
    */
  int lay;

  /**
    * @brief value of potential coordinate
    */
  double potential;
 };

}
#endif
