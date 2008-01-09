# Set the project's name
PROJECT( langmuir )

set(QT_MIN_VERSION "4.3.0") # we need at least version 4.3 of Qt
find_package(Qt4 REQUIRED) # find and setup Qt4 for this project

# Find and set up MPI
find_package(MPI REQUIRED)

include_directories(
	${CMAKE_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${MPI_INCLUDE_PATH}
)

# Set our SRCS (.cpp files)
SET( SRCS
	./rand.cpp
	./cubicgrid.cpp
	./hoppingagent.cpp
	./sourceagent.cpp
	./drainagent.cpp
	./simulation.cpp
)

SET( TSRCS
	./rand.cpp
	./cubicgrid.cpp
	./hoppingagent.cpp
	./sourceagent.cpp
	./drainagent.cpp
	./simulation.cpp
	./test.cpp
)

qt4_automoc(${avogadro_SRCS})

# Enable warnings
ADD_DEFINITIONS ( -Wall )

# Add the executables
ADD_EXECUTABLE( langmuir ./mpitest.cpp ${SRCS} )
ADD_EXECUTABLE( test ./test.cpp ${SRCS} )

# Libraries the executable needs to link to
TARGET_LINK_LIBRARIES( langmuir ${MPI_LIBRARY} )

TARGET_LINK_LIBRARIES( test ${QT_LIBRARIES} )
