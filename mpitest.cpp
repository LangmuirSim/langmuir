#include "cubicgrid.h"
#include "simulation.h"
#include "rand.h"
#include "cubicgrid.h"

#include <iostream>
#include <boost/mpi.hpp>
#include <boost/serialization/string.hpp>

#include <QDebug>

using namespace Langmuir;

namespace mpi = boost::mpi;

int main(int argc, char *argv[])
{
  mpi::environment env(argc, argv);
  mpi::communicator world;

  std::cout << "I am process " << world.rank() << " of " << world.size()
            << "." << std::endl;

  if (world.rank() == 0) {
    world.send(1, 0, std::string("Hello"));
    std::string msg;
    world.recv(1, 1, msg);
    std::cout << msg << "!" << std::endl;
  }
  else {
    std::string msg;
    world.recv(0, 0, msg);
    std::cout << msg << ", ";
    std::cout.flush();
    world.send(0, 1, std::string("world"));

  // Running tests in the second process...
  CubicGrid test(5, 5);

  int site1 = 20;
  int site2 = 21;

  qDebug() << "Distance between sites:" << test.getTotalDistance(site1, site2)
           << "," << test.getXDistance(site1, site2) << ","
           << test.getYDistance(site1, site2) << "\n";

  std::vector<int> neighbors;
  qDebug() << "neighbors.size()" << neighbors.size();

  neighbors = test.neighbors(site1);
  qDebug() << "neighbors.size()" << neighbors.size();
  std::cout << std::endl;
  std::cout << "site1 neighbors" << std::endl;
  for (int i = 0; i < neighbors.size(); ++i)
    std::cout << i << "\t" << neighbors[i] << std::endl;
  std::cout << std::endl;

  neighbors = test.neighbors(site2);
  std::cout << "site2 neighbors" << std::endl;
  for (int i = 0; i < neighbors.size(); ++i)
    std::cout << i << "\t" << neighbors[i] << std::endl;
  std::cout << std::endl;
  }

  // Test out the random number generator...
  Rand randGen(0.0, 1.0);
  double sum(0.0);
  int its = 10000000;
  int zero(0), one(0), two(0), three(0);
  for (int i = 0; i < its; ++i) {
    double num = randGen.number() * 3.99999999999;
    sum += num;
    int integer(num);
    if (integer == 0) ++zero;
    else if (integer == 1) ++one;
    else if (integer == 2) ++two;
    else if (integer == 3) ++three;
    else std::cout << "Error, unexpected number " << integer << std::endl;
  }
  std::cout << "Average number " << sum / its << " out of " << its << std::endl;
  std::cout << "0: " << zero << "\n1: " << one << "\n2: " << two << "\n3: " << three << std::endl;

  return 0;
/*
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
    if (rank == 0)
    {
        double f[5];
        f[0] = 5.5;
        COMM_WORLD.Send(f, 1, DOUBLE, 1, 55);
    }
    else
    {
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
        << ", " << test.getXDistance(site1, site2) << ", "
        << test.getYDistance(site1, site2) << "\n";

    // Create a 16x16 grid with a source potential of -5 and a drain potential of 0
    Simulation sim(16, 16, -5, 0);
    sim.performIterations(100);
*/
}
