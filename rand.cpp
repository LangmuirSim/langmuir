#include "rand.h"

#include <cstdlib>
#include <cmath>

using namespace std;

namespace Langmuir{

Rand::Rand()
{
	seed();
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
