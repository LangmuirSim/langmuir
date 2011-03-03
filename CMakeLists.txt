# Set the project's name
project(langmuir)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
cmake_minimum_required(VERSION 2.6.2)

#Find and setup Eigen2
find_package(Eigen2 REQUIRED)

#Find and set up MPI
#find_package(MPI REQUIRED)
#message(STATUS "MPI_COMPILE_FLAGS: ${MPI_COMPILE_FLAGS}")
#message(STATUS "MPI_INCLUDE_PATH: ${MPI_INCLUDE_PATH}")
#message(STATUS "MPI_LINK_FLAGS: ${MPI_LINK_FLAGS}")
#message(STATUS "MPI_LIBRARIES: ${MPI_LIBRARIES}")

#Find and set up Boost
#find_package(Boost REQUIRED COMPONENTS mpi)
find_package(Boost REQUIRED)

set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

#Find and set up QT4
set(QT_MIN_VERSION "4.5.0")
find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})

#Set include directories
include_directories(
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/..
  ${Boost_INCLUDE_DIRS}
  ${BOOST_PYTHON_INCLUDES}
  ${EIGEN2_INCLUDE_DIR}
)

#Set our SRCS (.cpp files)
set(SRCS
  rand.cpp
  cubicgrid.cpp
  sourceagent.cpp
  drainagent.cpp
  chargeagent.cpp
  simulation.cpp
  world.cpp
  inputparser.cpp
  linearpotential.cpp
)

#set(VIEWSRCS
#  rand.cpp
#  cubicgrid.cpp
#)
#set(MPISRCS
#  mpitest.cpp
#)
#set(qt_SRCS
#  gridview.cpp
#)
#qt4_automoc(${qt_SRCS})

# Enable warnings
add_definitions(-Wall)

# Add the executables
#add_executable(langmuirview langmuirview.cpp ${VIEWSRCS} ${qt_SRCS})
add_executable(test test.cpp ${qt_SRCS} ${SRCS})
#add_executable(test2 test_no_coulomb.cpp ${qt_SRCS} ${SRCS})
#add_executable(test3 test_coulomb.cpp ${qt_SRCS} ${SRCS})
#add_executable(test4 test_coulomb_drain.cpp ${qt_SRCS} ${SRCS})
#add_executable(test5 test_no_coulomb_drain.cpp ${qt_SRCS} ${SRCS})
#add_executable(test6 test_no_coulomb_vacs.cpp ${qt_SRCS} ${SRCS})
#add_executable(test7 test_coulomb_vacs.cpp ${qt_SRCS} ${SRCS})
#add_executable(mpitest ${qt_SRCS} ${MPISRCS} ${SRCS})
#add_executable(qtconcurrent qtconcurrent.cpp)

add_executable(langmuir langmuir.cpp ${SRCS})

target_link_libraries(langmuir
  ${QT_LIBRARIES}
  ${Boost_LIBRARIES})

# Libraries the executable needs to link to
#target_link_libraries(langmuirview 
#  ${MPI_LIBRARIES}
#  ${Boost_LIBRARIES}
#  ${QT_LIBRARIES})
target_link_libraries(test
  ${QT_LIBRARIES}
  ${Boost_LIBRARIES})
#target_link_libraries(test2
#  ${QT_LIBRARIES})
#target_link_libraries(test3
#  ${QT_LIBRARIES})
#target_link_libraries(test4
#  ${QT_LIBRARIES})
#target_link_libraries(test5
#  ${QT_LIBRARIES})
#target_link_libraries(test6
#  ${QT_LIBRARIES})
#target_link_libraries(test7
#  ${QT_LIBRARIES})
#target_link_libraries(mpitest
#  ${MPI_LIBRARIES}
#  ${Boost_LIBRARIES}
#  ${QT_LIBRARIES})
#target_link_libraries(qtconcurrent ${QT_LIBRARIES})
