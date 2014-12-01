# LANGMUIR #
* This is the source code for the **Langmuir** engine for charge transfer
  simulations in organic electronics.
* The source code is available under the GNU General Public License, v2 or later.
* Academic use should cite relevant papers, e.g.:

 1. Marcus D. Hanwell, Tamika A. Madison, Geoffrey R. Hutchison. “Charge Transport in Imperfect Organic Field Effect Transistors: Effects of Explicit Defects and Electrostatics.” J. Phys. Chem. C 2010 114(48) 20417-20423. http://dx.doi.org/10.1021/jp104416a
 2. Tamika A. Madison, Adam G. Gagorik, Geoffrey R. Hutchison. “Charge Transport in Imperfect Organic Field Effect Transistors: Effects of Charge Traps.” J. Phys. Chem. C 2012 116(22) 11852-11858. http://dx.doi.org/10.1021/jp207421n
 3. Adam G. Gagorik, Geoffrey R. Hutchison. “Simulating Charge Injection and Dynamics in Micro-Scale Organic Field-Effect Transistors.” J. Phys. Chem. C 2012 116(40) 21232-21239. http://dx.doi.org/10.1021/jp306597n
 4. Adam G. Gagorik, Jacob W. Mohin, Tomasz Kowalewski, Geoffrey R. Hutchison. “Monte Carlo Simulations of Charge Transport in 2D Organic Photovoltaics.” J. Phys. Chem. Lett 2013 4(1) 36-42. http://dx.doi.org/10.1021/jz3016292
 5. Adam G. Gagorik, Jacob W. Mohin, Tomasz Kowalewski, Geoffrey R. Hutchison. “Effects of Delocalized Charges on Organic Photovoltaics: Nanoscale 3D Monte Carlo Simulations.” Adv. Func. Mater. 2014. http://dx.doi.org/10.1002/adfm.201402332

## BUILD INSTRUCTIONS ##
1. In order to build the Langmuir engine the following dependencies are required:
    * Qt4
    * Boost
    * CMake
2. The following are optional:
    * OpenCL 1.1
    * OpenGL
    * Doxygen
3. Simple build:
    * mkdir build
    * cd build
    * cmake ../
    * make -j 4
4. On a MAC, you may need to declare the arch:
    *  cmake -DCMAKE_OSX_ARCHITECTURES=x86_64 ../
5. To use included OPENCL 1.1 headers:
    *  cmake -DOPENCL_INCLUDE_DIRS=..
6. To make the documentation:
    * make doc
    * make show (opens doc/html/index.html in a web browser)
7. On a cluster with modules, you have to set things up:
    * module purge
    * module load cmake
    * module load gcc
    * module load boost/1.50.0-gcc45
    * module load qt
    * module list
8. To perform clang scan-build:
    * mkdir build
    * cd build
    * scan-build -v cmake ..
    * scan-build -v -k -analyze-headers -stats -o . make -j 4
    * scan-view scan-build-output-dir
    * scan-build-output-dir will be in the current directory and have the current date for its name
