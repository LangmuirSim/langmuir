#include "rand.h"

#include <cstdlib>
#include <cmath>

using namespace std;

namespace Langmuir{

Rand::Rand()
{
	seed();
}

Rand::Rand(double min, double max)
{
	m_min = min;
	m_range = max - min;
}

double Rand::number()
{
	// Generate a random number between min and max...
	return m_range * (double(rand()) / double(RAND_MAX)) + m_min;
}

double Rand::number(double min, double max)
{
	// Generate a random number between min and max...
	double range = max - min;
	return range * (double(rand()) / double(RAND_MAX)) + min;
}

void Rand::seed()
{
	sranddev();
}

} // End namespace Langmuir
