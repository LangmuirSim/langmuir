#ifndef _TIMER_LANGMUIR_H
#define _TIMER_LANGMUIR_H

#include <QtCore>
#include <sys/time.h>

namespace Langmuir
{

  class Timer : public QObject
  {

  private:
      Q_OBJECT
      Q_DISABLE_COPY(Timer)

  public:

    Timer ();
    ~Timer ();

    double elapsed ();
    double restart ();

    double elapsed (double since);
    double now ();

  private:

    struct timeval timev;
    struct timezone timez;
    double start;

  };

  inline Timer::Timer ()
  {
    gettimeofday (&timev, &timez);
    start = (timev.tv_sec + timev.tv_usec / 1000000.0);
  }

  inline Timer::~Timer ()
  {

  }

  inline double Timer::elapsed ()
  {
    gettimeofday (&timev, &timez);
    return ((timev.tv_sec + timev.tv_usec / 1000000.0) - start);
  }

  inline double Timer::elapsed (double since)
  {
    gettimeofday (&timev, &timez);
    return ((timev.tv_sec + timev.tv_usec / 1000000.0) - since);
  }

  inline double Timer::restart ()
  {
    gettimeofday (&timev, &timez);
    start = (timev.tv_sec + timev.tv_usec / 1000000.0);
    return start;
  }

  inline double Timer::now ()
  {
    gettimeofday (&timev, &timez);
    return (timev.tv_sec + timev.tv_usec / 1000000.0);
  }

}
#endif
