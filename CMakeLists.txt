project( langmuir )
set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )
cmake_minimum_required( VERSION 2.8.2 )

set( CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG" )
set( QT_MIN_VERSION "4.7.0" )
set( QT_USE_QTOPENGL TRUE )

set( sources
      rand.cpp
      cubicgrid.cpp
      sourceagent.cpp
      drainagent.cpp
      chargeagent.cpp
      simulation.cpp
      world.cpp
      inputparser.cpp
      linearpotential.cpp )

set( meta_object_headers
      gridview.h )

find_package( Eigen2 REQUIRED )
find_package( Boost REQUIRED )
find_package( Qt4 REQUIRED )
find_package( OpenCL REQUIRED )
find_package( OpenGL REQUIRED )

include( ${QT_USE_FILE} )
include_directories( ${EIGEN2_INCLUDE_DIR} )
include_directories( ${Boost_INCLUDE_DIRS} )
include_directories( ${BOOST_PYTHON_INCLUDES} )
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${OPENCL_INCLUDE_DIRS} )

add_definitions( -Wall )
add_definitions( ${QT_DEFINITIONS} )

add_executable( langmuir langmuir.cpp ${sources} ) 
target_link_libraries( langmuir ${EIGEN2_LIBRARIES} )
target_link_libraries( langmuir ${Boost_LIBRARIES} )
target_link_libraries( langmuir ${QT_LIBRARIES} )
target_link_libraries( langmuir ${OPENCL_LIBRARIES} )

QT4_WRAP_CPP( moc_output ${meta_object_headers} )
include_directories( ${OPENGL_INCLUDE_DIRS} )
add_executable( langmuirView langmuirview.cpp gridview.cpp ${sources} ${moc_output} )
target_link_libraries( langmuirView ${EIGEN2_LIBRARIES} )
target_link_libraries( langmuirView ${Boost_LIBRARIES} )
target_link_libraries( langmuirView ${QT_LIBRARIES} )
target_link_libraries( langmuirView ${OPENCL_LIBRARIES} )
target_link_libraries( langmuirView ${OPENGL_LIBRARIES} )
