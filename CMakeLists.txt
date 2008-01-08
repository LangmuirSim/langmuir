# Set the project's name
PROJECT( langmuir )

# Find and set up MPI
find_package(MPI REQUIRED)

include_directories(
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${MPI_INCLUDE_PATH}
)

# Set our SRCS (.cpp files)
SET( SRCS
  ./cubicgrid.cpp
  ./hoppingagent.cpp
  ./sourceagent.cpp
  ./drainagent.cpp
  ./simulation.cpp
  ./mpitest.cpp
)

# Enable warnings
ADD_DEFINITIONS ( -Wall )

# Add the executables
ADD_EXECUTABLE( langmuir ${SRCS} )

# Libraries the executable needs to link to
TARGET_LINK_LIBRARIES( langmuir ${MPI_LIBRARY} )
