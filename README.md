# LANGMUIR #
* This is the source code for the **Langmuir** engine for charge transfer
  simulations in organic electronics.

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
