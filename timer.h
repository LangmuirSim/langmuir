#ifndef _TIMER_LANGMUIR_H
#define _TIMER_LANGMUIR_H

#include <sys/time.h>

namespace Langmuir
{

  class Timer
  {

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

    Timer::Timer ()
  {
    gettimeofday (&timev, &timez);
    start = (timev.tv_sec + timev.tv_usec / 1000000.0);
  }

  Timer::~Timer ()
  {

  }

  double Timer::elapsed ()
  {
    gettimeofday (&timev, &timez);
    return ((timev.tv_sec + timev.tv_usec / 1000000.0) - start);
  }

  double Timer::elapsed (double since)
  {
    gettimeofday (&timev, &timez);
    return ((timev.tv_sec + timev.tv_usec / 1000000.0) - since);
  }

  double Timer::restart ()
  {
    gettimeofday (&timev, &timez);
    start = (timev.tv_sec + timev.tv_usec / 1000000.0);
    return start;
  }

  double Timer::now ()
  {
    gettimeofday (&timev, &timez);
    return (timev.tv_sec + timev.tv_usec / 1000000.0);
  }

}
#endif
