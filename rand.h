/*
 *  Rand - agent to act as a source of charges
 */

#ifndef __RAND_H
#define __RAND_H

namespace Langmuir
{

class Rand
{
public:
	Rand();
	Rand(double min, double max);
	double number();
	double number(double min, double max);

private:
	void seed();
	double m_min;
	double m_range;
};

} // End namespace Langmuir

#endif
