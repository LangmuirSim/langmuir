#include "linearpotential.h"

namespace Langmuir
{

 LinearPotential::LinearPotential()
 {
 }

 LinearPotential::LinearPotential( QVector<PotentialPoint>& PotentialPoints )
 {
  for ( int i = 0; i < PotentialPoints.size(); i++ )
  {
   addPoint(PotentialPoints[i]);
  }
 }

 LinearPotential::~LinearPotential()
 {
 }

 void LinearPotential::addPoint( PotentialPoint point )
 {

  if ( point.onOrigin() )
  {
   qFatal("Do not define potential points on origin... can not determine axis.");
  }
  else if ( point.onXAxis() )
  {
   addXPoint(point);
  }
  else if ( point.onYAxis() )
  {
   addYPoint(point);
  }
  else if ( point.onZAxis() )
  {
   addZPoint(point);
  }
  else
  {
   qFatal("Linear potential specified but point not defined on an axis. In addPoint.");
  }
 }

 void LinearPotential::addSourcePoint( PotentialPoint point )
 {

  if ( !(point.x() == 0) )
  {
   qFatal("Linear potential of source specified is invalid.");
  }

  if ( !(point.y() == 0) )
  {
   qFatal("Linear potential of source specified is invalid.");
  }

  if ( !(point.z() == 0) )
  {
   qFatal("Linear potential of source specified is invalid.");
  }

  addXPoint(point);

 }

 void LinearPotential::addDrainPoint( PotentialPoint point )
 {

  if ( !(point.y() == 0) )
  {
   qFatal("Linear potential of drain specified is invalid.");
  }

  if ( !(point.z() == 0) )
  {
   qFatal("Linear potential of drain specified is invalid.");
  }

  addXPoint(point);

 }

 double LinearPotential::calculate( double col, double row, double lay )
 {

  double potential = 0;
  bool defined = false;

  //Is a region even defined
  if ( xx.size() >= 2 )
  {
   int x = regionX(col);
   potential += mx[x]*col+bx[x];
   defined = true;
  }

  //Is a region even defined
  if ( yy.size() >= 2 )
  {
  int y = regionY(row);
  potential += my[y]*row+by[y];
  defined = true;
  }

  //Is a region even defined
  if ( zz.size() >= 2 )
  {
  int z = regionZ(lay);
  potential += mz[z]*lay+bz[z];
  defined = true;
  }

  if ( !defined ) qFatal("not enough points defined to calculate potential");

  return potential;
 }

 int LinearPotential::regionX( double x )
 {
  //Below the range of defined points
  if ( x < xx[0].x() ) return 0;

  //Above the range of defined points
  if ( x > xx[xx.size()-1].x() ) return mx.size()-1;

  //In the range of defined points
  int region = 0;
  while ( x > xx[region+1].x() ) region += 1;

  return region;
 }

 int LinearPotential::regionY( double y )
 {
  //Below the range of defined points
  if ( y < yy[0].y() ) return 0;

  //Above the range of defined points
  if ( y > yy[yy.size()-1].y() ) return my.size()-1;

  //In the range of defined points
  int region = 0;
  while ( y > yy[region+1].y() ) region += 1;

  return region;
 }

 int LinearPotential::regionZ( double z )
 {
  //Below the range of defined points
  if ( z < zz[0].z() ) return 0;

  //Above the range of defined points
  if ( z > zz[zz.size()-1].z() ) return mz.size()-1;

  //In the range of defined points
  int region = 0;
  while ( z > zz[region+1].z() ) region += 1;

  return region;
 }

 void LinearPotential::addXPoint( PotentialPoint& p )
 {
  addXPoint(p.x(),p.V());
 }

 void LinearPotential::addYPoint( PotentialPoint& p )
 {
  addYPoint(p.y(),p.V());
 }

 void LinearPotential::addZPoint( PotentialPoint& p )
 {
  addZPoint(p.z(),p.V());
 }

 void LinearPotential::addXPoint( double col, double potential )
 {
  double deltaX    = 0;
  double deltaY    = 0;
  double slope     = 0;
  double intercept = 0;

  //Create the point
  PotentialPoint p(col,0,0,potential);

  //Check if there are other points present
  if ( xx.size() >= 1 )
  {
   //The added point is less than the previous point in list - sorting must be done
   if ( PotentialPoint::less_than_by_x_value()(p,xx[xx.size()-1]) )
   {
    //Add the point
    xx.push_back(p);
    //Clear the slopes - they need recomputed
    mx.clear();
    //Clear the intercepts - they need recomputed
    bx.clear();
    //Sort the points
    std::sort(xx.begin(),xx.end(),PotentialPoint::less_than_by_x_value());
    //Recalculate the slopes and the intercepts
    for ( int i = 1; i < xx.size(); i++ )
    {
     deltaX = double(xx[i].x()) - double(xx[i-1].x());
     deltaY =       (xx[i].V()  -        xx[i-1].V());
     //Is it a horizontal line
     if ( abs(deltaY) < 10e-10 )
     {
      slope = 0;
      intercept = xx[i-1].V();
     } 
     //Is it a vertical line - theres an error if so
     else if ( abs(deltaX) < 10e-10 )
     {
      throw( std::length_error("Can not calculate potential - deltaX too small") );
     }
     //Is it a slanted line
     else 
     {
      slope = deltaY / deltaX;
      intercept = xx[i-1].V() - slope * double(xx[i-1].x());
     }
     //Add the slope
     mx.push_back(slope);
     //Add the intercept
     bx.push_back(intercept);
    }
   }
   //The added point is greather than the previous point in list - no sorting needed
   else
   {
    //Compute the slope and intercept between this point and the previous
    deltaX = double(xx[xx.size()-1].x()) - double(p.x());
    deltaY = xx[xx.size()-1].V() - p.V();
    //Is it a horizontal line
    if ( abs(deltaY) < 10e-10 )
    {
     slope = 0;
     intercept = xx[xx.size()-1].V();
    }
    //Is it a vertical line - theres an error if so
    else if ( abs(deltaX) < 10e-10 )
    {
     throw( std::length_error("Can not calculate potential - deltaX too small") );
    }
    //Is it a slanted line
    else 
    {
     slope = deltaY / deltaX;
     intercept = xx[xx.size()-1].V() - slope * double(xx[xx.size()-1].x());
    }
    //Add the point
    xx.push_back(p);
    //Add the slope
    mx.push_back(slope);
    //Add the intercept
    bx.push_back(intercept);
   }
  }
  //This is the first point added - no slopes, intercepts, or sorting needed
  else
  {
   //Add the point
   xx.push_back(p);
  }
 }

 void LinearPotential::addYPoint( double row, double potential )
 {
  double deltaX    = 0;
  double deltaY    = 0;
  double slope     = 0;
  double intercept = 0;

  //Create the point
  PotentialPoint p(0,row,0,potential);

  //Check if there are other points present
  if ( yy.size() >= 1 )
  {
   //The added point is less than the previous point in list - sorting must be done
   if ( PotentialPoint::less_than_by_y_value()(p,yy[yy.size()-1]) )
   {
    //Add the point
    yy.push_back(p);
    //Clear the slopes - they need recomputed
    my.clear();
    //Clear the intercepts - they need recomputed
    by.clear();
    //Sort the points
    std::sort(yy.begin(),yy.end(),PotentialPoint::less_than_by_y_value());
    //Recalculate the slopes and the intercepts
    for ( int i = 1; i < yy.size(); i++ )
    {
     deltaX = double(yy[i].y()) - double(yy[i-1].y());
     deltaY =       (yy[i].V()  -        yy[i-1].V());
     //Is it a horizontal line
     if ( abs(deltaY) < 10e-10 )
     {
      slope = 0;
      intercept = yy[i-1].V();
     } 
     //Is it a vertical line - theres an error if so
     else if ( abs(deltaX) < 10e-10 )
     {
      throw( std::length_error("Can not calculate potential - deltaX too small") );
     }
     //Is it a slanted line
     else 
     {
      slope = deltaY / deltaX;
      intercept = yy[i-1].V() - slope * double(yy[i-1].y());
     }
     //Add the slope
     my.push_back(slope);
     //Add the intercept
     by.push_back(intercept);
    }
   }
   //The added point is greather than the previous point in list - no sorting needed
   else
   {
    //Compute the slope and intercept between this point and the previous
    deltaX = double(yy[yy.size()-1].y()) - double(p.y());
    deltaY = yy[yy.size()-1].V() - p.V();
    //Is it a horizontal line
    if ( abs(deltaY) < 10e-10 )
    {
     slope = 0;
     intercept = yy[yy.size()-1].V();
    }
    //Is it a vertical line - theres an error if so
    else if ( abs(deltaX) < 10e-10 )
    {
     throw( std::length_error("Can not calculate potential - deltaX too small") );
    }
    //Is it a slanted line
    else 
    {
     slope = deltaY / deltaX;
     intercept = yy[yy.size()-1].V() - slope * double(yy[yy.size()-1].y());
    }
    //Add the point
    yy.push_back(p);
    //Add the slope
    my.push_back(slope);
    //Add the intercept
    by.push_back(intercept);
   }
  }
  //This is the first point added - no slopes, intercepts, or sorting needed
  else
  {
   //Add the point
   yy.push_back(p);
  }
 }

 void LinearPotential::addZPoint( double lay, double potential )
 {
  double deltaX    = 0;
  double deltaY    = 0;
  double slope     = 0;
  double intercept = 0;

  //Create the point
  PotentialPoint p(0,0,lay,potential);

  //Check if there are other points present
  if ( zz.size() >= 1 )
  {
   //The added point is less than the previous point in list - sorting must be done
   if ( PotentialPoint::less_than_by_z_value()(p,zz[zz.size()-1]) )
   {
    //Add the point
    zz.push_back(p);
    //Clear the slopes - they need recomputed
    mz.clear();
    //Clear the intercepts - they need recomputed
    bz.clear();
    //Sort the points
    std::sort(zz.begin(),zz.end(),PotentialPoint::less_than_by_z_value());
    //Recalculate the slopes and the intercepts
    for ( int i = 1; i < zz.size(); i++ )
    {
     deltaX = double(zz[i].z()) - double(zz[i-1].z());
     deltaY =       (zz[i].V()  -        zz[i-1].V());
     //Is it a horizontal line
     if ( abs(deltaY) < 10e-10 )
     {
      slope = 0;
      intercept = zz[i-1].V();
     } 
     //Is it a vertical line - theres an error if so
     else if ( abs(deltaX) < 10e-10 )
     {
      throw( std::length_error("Can not calculate potential - deltaX too small") );
     }
     //Is it a slanted line
     else 
     {
      slope = deltaY / deltaX;
      intercept = zz[i-1].V() - slope * double(zz[i-1].z());
     }
     //Add the slope
     mz.push_back(slope);
     //Add the intercept
     bz.push_back(intercept);
    }
   }
   //The added point is greather than the previous point in list - no sorting needed
   else
   {
    //Compute the slope and intercept between this point and the previous
    deltaX = double(zz[zz.size()-1].z()) - double(p.z());
    deltaY = zz[zz.size()-1].V() - p.V();
    //Is it a horizontal line
    if ( abs(deltaY) < 10e-10 )
    {
     slope = 0;
     intercept = zz[zz.size()-1].V();
    }
    //Is it a vertical line - theres an error if so
    else if ( abs(deltaX) < 10e-10 )
    {
     throw( std::length_error("Can not calculate potential - deltaX too small") );
    }
    //Is it a slanted line
    else 
    {
     slope = deltaY / deltaX;
     intercept = zz[zz.size()-1].V() - slope * double(zz[zz.size()-1].z());
    }
    //Add the point
    zz.push_back(p);
    //Add the slope
    mz.push_back(slope);
    //Add the intercept
    bz.push_back(intercept);
   }
  }
  //This is the first point added - no slopes, intercepts, or sorting needed
  else
  {
   //Add the point
   zz.push_back(p);
  }
 }

 void LinearPotential::plot( QTextStream& stream, double xstart, double xstop, double ystart, double ystop, double zvalue, double xdelta, double ydelta )
 {
  for ( double x = xstart; x <= xstop; x += xdelta )
  {
   for ( double y = ystart; y <= ystop; y += ydelta )
   {
    stream << x << " " << y << " " << calculate(x,y,zvalue) << "\n";
   }
   stream << "\n";
  }
 }
}
