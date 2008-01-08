/*
 *  Rand - agent to act as a source of charges
 */

#ifndef __RAND_H
#define __RAND_H

#include "agent.h"

#include <vector>

namespace Langmuir
{

class Rand
{
public:
	Rand();
	double number(double min = 0, double max = 1.0);
	
private:
	void seed();
	
};

} // End namespace Langmuir

#endif
