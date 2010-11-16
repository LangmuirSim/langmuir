#ifndef _POTENTIALPOINT_H
#define _POTENTIALPOINT_H

#include<iostream>
#include<cmath>
#include<stdexcept>
#include<QTextStream>

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
  PotentialPoint( double col, double row, double lay, double potential )
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
    * @brief operator << overload 
    *
    * Allows the expression: ostream << PotentialPoint;
    */
  friend QTextStream& operator<<(QTextStream& qt, const PotentialPoint& p)
  {
   qt.setFieldWidth(1);
   qt <<   "(";
   qt << p.col;
   qt <<   ",";
   qt << p.row;
   qt <<   ",";
   qt << p.lay;
   qt <<   ")";
   return qt;
  }

  /**
    * @struct comparison functor
    *
    * Used for sorting points with standard library.
    */
  struct less_than_by_x_value
  {
   /**
     * @brief functor comparison
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
     * @brief functor comparison
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
     * @brief functor comparison
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
 inline const double& x() const { return col; }

 /**
   * @brief access row value
   * @return row value
   */
 inline const double& y() const { return row; }

 /**
   * @brief access lay value
   * @return lay value
   */
 inline const double& z() const { return lay; }

 /**
   * @brief access potential value
   * @return potential value
   */
 inline const double& V() const { return potential; }

 /**
   * @brief decide if this point lies on the x axis
   * @return boolean true of false
   */
 inline bool onXAxis() const 
 { 
  if ( std::abs(row) > 10e-10 ) return false;
  if ( std::abs(lay) > 10e-10 ) return false;
             return  true;
 }

 /**
   * @brief decide if this point lies on the y axis
   * @return boolean true of false
   */
 inline bool onYAxis() const 
 { 
  if ( std::abs(col) > 10e-10 ) return false;
  if ( std::abs(lay) > 10e-10 ) return false;
             return  true;
 }

 /**
   * @brief decide if this point lies on the z axis
   * @return boolean true of false
   */
 inline bool onZAxis() const 
 { 
  if ( std::abs(row) > 10e-10 ) return false;
  if ( std::abs(col) > 10e-10 ) return false;
             return  true;
 }

 /**
   * @brief decide if this point is the origin
   * @return boolean true of false
   */
 inline bool onOrigin() const 
 { 
  if ( std::abs(row) > 10e-10 ) return false;
  if ( std::abs(col) > 10e-10 ) return false;
  if ( std::abs(lay) > 10e-10 ) return false;
             return  true;
 }

 protected:
  /**
    * @brief value of x coordinate 
    */
  double col;

  /** 
    * @brief value of y coordinate
    */
  double row;

  /**
    * @brief value of z coordinate
    */
  double lay;

  /**
    * @brief value of potential coordinate
    */
  double potential;
 };
#endif
