#include <iostream>
#include <mpi.h>

#include "cubicgrid.h"
#include "simulation.h"

using namespace std;
using namespace MPI;
using namespace Langmuir;

int main(int argc, char *argv[]) {
  int numprocs, rank, namelen;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  
  Init(argc, argv);
  rank = COMM_WORLD.Get_rank();
  numprocs = COMM_WORLD.Get_size();
//  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
//  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  COMM_WORLD.Get_name(processor_name, namelen);

  // Create a status object
  Status info;

  // Now send some data
  if (rank == 0) {
    double f[5];
    f[0] = 5.5;
    COMM_WORLD.Send(f, 1, DOUBLE, 1, 55);
  }
  else {
    double value[5];
    COMM_WORLD.Recv(value, 1, DOUBLE, 0, 55);
    cout << "Value: " << value[0] << "\n";
  }
  
  cout << "Process " << rank << " on " << processor_name << " out of "
    << numprocs << ".\n";
  
  if (rank == 0)
    cout << "Master node.\n";
  else
    cout << "Worker node.\n";
  
  MPI_Finalize();
  
  cout << "Testing cubic grid class...\n";
  CubicGrid test(10, 10);
  
  int site1 = 5;
  int site2 = 66;
  
  cout << "Distance between sites: " << test.getTotalDistance(site1, site2)
       << ", " << test.getXDistance(site1, site2)
       << ", " << test.getYDistance(site1, site2)
       << "\n";
  
  Simulation sim(16, 16);
}
