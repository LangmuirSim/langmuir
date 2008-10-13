# Set the project's name
PROJECT(langmuir)

cmake_minimum_required(VERSION 2.6)

# Find and set up MPI
#find_package(MPI REQUIRED)

# Also need random number facilities from Boost
find_package(Boost REQUIRED
  COMPONENTS random)

message(STATUS "MPI_COMPILE_FLAGS: ${MPI_COMPILE_FLAGS}")
message(STATUS "MPI_INCLUDE_PATH: ${MPI_INCLUDE_PATH}")
message(STATUS "MPI_LINK_FLAGS: ${MPI_LINK_FLAGS}")
message(STATUS "MPI_LIBRARIES: ${MPI_LIBRARIES}")

set(QT_MIN_VERSION "4.3.4") # we need at least version 4.3 of Qt
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
)

# Set our SRCS (.cpp files)
SET(SRCS
	rand.cpp
	cubicgrid.cpp
	hoppingagent.cpp
	sourceagent.cpp
	drainagent.cpp
	simulation.cpp
)

SET(TSRCS
	rand.cpp
	cubicgrid.cpp
	hoppingagent.cpp
	sourceagent.cpp
	drainagent.cpp
	simulation.cpp
	test.cpp
)

SET(qt_SRCS
  gridview.cpp)

qt4_automoc(${qt_SRCS})

# Enable warnings
ADD_DEFINITIONS ( -Wall )

# Add the executables
#ADD_EXECUTABLE( langmuir mpitest.cpp ${SRCS} )
ADD_EXECUTABLE( test test.cpp ${qt_SRCS} ${SRCS} )

# Libraries the executable needs to link to
#TARGET_LINK_LIBRARIES( langmuir ${MPI_LIBRARIES}
#  ${Boost_LIBRARIES})

TARGET_LINK_LIBRARIES( test ${QT_LIBRARIES}
  ${Boost_LIBRARIES})
