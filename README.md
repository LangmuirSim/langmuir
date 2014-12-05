# LANGMUIR #
* This is the source code for the **Langmuir** engine for charge transfer simulations in organic electronics.
* The source code is available under the GNU General Public License, v2 or later.
* Academic use should cite relevant papers, e.g.:

 1. Marcus D. Hanwell, Tamika A. Madison, Geoffrey R. Hutchison. “Charge Transport in Imperfect Organic Field Effect Transistors: Effects of Explicit Defects and Electrostatics.” J. Phys. Chem. C 2010 114(48) 20417-20423. http://dx.doi.org/10.1021/jp104416a
 2. Tamika A. Madison, Adam G. Gagorik, Geoffrey R. Hutchison. “Charge Transport in Imperfect Organic Field Effect Transistors: Effects of Charge Traps.” J. Phys. Chem. C 2012 116(22) 11852-11858. http://dx.doi.org/10.1021/jp207421n
 3. Adam G. Gagorik, Geoffrey R. Hutchison. “Simulating Charge Injection and Dynamics in Micro-Scale Organic Field-Effect Transistors.” J. Phys. Chem. C 2012 116(40) 21232-21239. http://dx.doi.org/10.1021/jp306597n
 4. Adam G. Gagorik, Jacob W. Mohin, Tomasz Kowalewski, Geoffrey R. Hutchison. “Monte Carlo Simulations of Charge Transport in 2D Organic Photovoltaics.” J. Phys. Chem. Lett 2013 4(1) 36-42. http://dx.doi.org/10.1021/jz3016292
 5. Adam G. Gagorik, Jacob W. Mohin, Tomasz Kowalewski, Geoffrey R. Hutchison. “Effects of Delocalized Charges on Organic Photovoltaics: Nanoscale 3D Monte Carlo Simulations.” Adv. Func. Mater. 2014. http://dx.doi.org/10.1002/adfm.201402332

## EXAMPLES ##
1. example input files can be found in ./examples
2. manual can be found in ./misc

## BUILD INSTRUCTIONS ##
1. In order to build the Langmuir engine the following dependencies are required:

 * Qt4
 * Boost
 * CMake

2. The following are optional:

 * OpenCL 1.1
 * OpenGL
 * Qt5

3. QtCreator build:

 * open ./src/CMakeLists.txt in QtCreator

4. Langmuir build:

 * cd ./src
 * mkdir build
 * cd build
 * cmake ../
 * make
 * exe is ./build/langmuir/langmuir

5. LangmuirView build:

 * need Qt5
 * make langmuirView
 * exe is ./build/langmuirView/langmuirView

6. Clang scan-build:

 * mkdir build
 * cd build
 * scan-build -v cmake ..
 * scan-build -v -k -analyze-headers -stats -o . make -j 4
 * scan-view scan-build-output-dir

## Notes ##

1. On a MAC, you may need to declare the arch:

 *  cmake -DCMAKE_OSX_ARCHITECTURES=x86_64 ../

2. On a cluster with modules, may have to set things up first:

 * module purge
 * module load cmake
 * module load gcc
 * module load boost/1.50.0-gcc45
 * module load qt
 * module list

## Python ##
1.  see ./LangmuirPython/README.md

## Documentation ##
1. To make the cxx documentation:

 * cd ./doc
 * make html
 * make latex

2. To make the manual:

 * cd ./doc/manual
 * make

3.  To make the python docs:

 * cd ./LangmuirPython/doc
 * export PYTHONPATH=$PYTHONPATH:/path/to/LangmuirPython
 * make html
 * make latexpdf
