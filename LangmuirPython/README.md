# Langmuir Python #
  * A set of python scripts for running and analyzing Langmuir simulations.

## Contents ##
  - langmuir : the main python package
  - utils    : scripts for running simulations
  - analyze  : scripts for analyzing output files
  - input    : scripts for creating input file
  - misc     : misc scripts

## Update Database ##
  * ./langmuir/database.py contains metadata about simulation parameters
  * If you add parameters to langmuir, you should update ./langmuir/database.py

## Install ##
  1.  install required modules:
    * numpy
    * scipy
    * matplotlib
    * pandas
    * quantities
  2.  install optional modules: vtk
  3.  Method A
    * add PYTHONPATH=$PYTHONPATH:/path/to/LangmuirPython to .bashrc
    * source .bashrc
  4.  Method B
    * python setup.py --install --user
  5.  Test if things work
    * from python, 'import langmuir' to see if it works
