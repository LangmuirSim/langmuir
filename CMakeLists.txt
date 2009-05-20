# Set the project's name
project(langmuir)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

cmake_minimum_required(VERSION 2.6.2)

find_package(Eigen2 REQUIRED) # find and setup Eigen2
# Find and set up MPI
#find_package(MPI REQUIRED)

# Also need random number facilities from Boost
find_package(Boost REQUIRED
  COMPONENTS random mpi)

message(STATUS "MPI_COMPILE_FLAGS: ${MPI_COMPILE_FLAGS}")
message(STATUS "MPI_INCLUDE_PATH: ${MPI_INCLUDE_PATH}")
message(STATUS "MPI_LINK_FLAGS: ${MPI_LINK_FLAGS}")
message(STATUS "MPI_LIBRARIES: ${MPI_LIBRARIES}")

set(QT_MIN_VERSION "4.5.0") # we need at least version 4.4 of Qt
find_package(Qt4 REQUIRED) # find and setup Qt4 for this project

include(${QT_USE_FILE})

include_directories(
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/..
  ${Boost_INCLUDE_DIRS}
  ${BOOST_PYTHON_INCLUDES}
)

include_directories(
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${EIGEN2_INCLUDE_DIR}
)

# Set our SRCS (.cpp files)
set(SRCS
  rand.cpp
  cubicgrid.cpp
  sourceagent.cpp
  drainagent.cpp
  chargeagent.cpp
  simulation.cpp
  world.cpp
)

set(VIEWSRCS
  rand.cpp
  cubicgrid.cpp
)

set(MPISRCS
  mpitest.cpp
)

set(qt_SRCS
  gridview.cpp)

qt4_automoc(${qt_SRCS})

# Enable warnings
add_definitions(-Wall)

# Add the executables
add_executable(langmuirview langmuirview.cpp ${VIEWSRCS} ${qt_SRCS})
add_executable(test test.cpp ${qt_SRCS} ${SRCS})

add_executable(mpitest ${qt_SRCS} ${MPISRCS} ${SRCS})

# Libraries the executable needs to link to
target_link_libraries(langmuirview #${MPI_LIBRARIES}
  ${Boost_LIBRARIES}
  ${QT_LIBRARIES})

target_link_libraries(test
  ${QT_LIBRARIES}
  ${Boost_LIBRARIES})

target_link_libraries(mpitest
#  ${MPI_LIBRARIES}
  ${Boost_LIBRARIES}
  ${QT_LIBRARIES})

add_executable(qtconcurrent qtconcurrent.cpp)
target_link_libraries(qtconcurrent ${QT_LIBRARIES})
