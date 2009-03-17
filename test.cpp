#include <iostream>
#include <unistd.h>

#include "cubicgrid.h"
#include "simulation.h"

using namespace std;
using namespace Langmuir;

int main(int argc, char *argv[])
{
	cout << "Testing cubic grid class...\n";
	CubicGrid test(10, 10);

	int site1 = 5;
	int site2 = 66;

	cout << "Distance between sites: " << test.getTotalDistance(site1, site2)
		<< ", " << test.getXDistance(site1, site2) << ", "
		<< test.getYDistance(site1, site2) << "\n";

	// Create a 16x16 grid with a source potential of -5 and a drain potential of 0
	cout << "Testing the simulation class.\n";
  Simulation sim(10, 5, 0, -1.0);

  for (int i = 0; i < 100; i++) {
    sim.printGrid();
    sim.performIterations(1);
    usleep(100000);
	}
}
