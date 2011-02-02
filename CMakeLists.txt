project( langmuir )
set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )
cmake_minimum_required( VERSION 2.8.2 )

set( CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG" )
set( QT_MIN_VERSION "4.7.0" )

find_package( Eigen2 REQUIRED )
find_package( Boost REQUIRED )
find_package( Qt4 REQUIRED )
find_package( OpenCL REQUIRED )

include( ${QT_USE_FILE} )
include_directories( ${CMAKE_SOURCE_DIR} )
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_CURRENT_BINARY_DIR}/.. )
include_directories( ${EIGEN2_INCLUDE_DIR} )
include_directories( ${Boost_INCLUDE_DIRS} )
include_directories( ${BOOST_PYTHON_INCLUDES} )
include_directories( ${OPENCL_INCLUDE_DIRS} )

add_definitions( -Wall )
add_definitions( ${QT_DEFINITIONS} )

add_executable( langmuir 
                 langmuir.cpp 
                 rand.cpp
                 cubicgrid.cpp 
                 sourceagent.cpp
                 drainagent.cpp
                 chargeagent.cpp
                 simulation.cpp
                 world.cpp
                 inputparser.cpp
                 linearpotential.cpp )

target_link_libraries( langmuir ${EIGEN2_LIBRARIES} )
target_link_libraries( langmuir ${Boost_LIBRARIES} )
target_link_libraries( langmuir ${QT_LIBRARIES} )
target_link_libraries( langmuir ${OPENCL_LIBRARIES} )
