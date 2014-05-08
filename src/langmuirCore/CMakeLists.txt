project(langmuirCore)
cmake_minimum_required(VERSION 2.8)

message(STATUS "Project: ${PROJECT_NAME}")
set(CMAKE_BUILD_TYPE Debug)

set(SOURCES
        rand.cpp
        gzipper.cpp
        nodefileparser.cpp
        clparser.cpp

        world.cpp
        simulation.cpp
        potential.cpp
        cubicgrid.cpp
        openclhelper.cpp
        keyvalueparser.cpp

        chargeagent.cpp
        fluxagent.cpp
        drainagent.cpp
        sourceagent.cpp

        output.cpp
        writer.cpp
        checkpointer.cpp
)

set(HEADERS
        ./include/rand.h
        ./include/gzipper.h
        ./include/nodefileparser.h
        ./include/clparser.h

        ./include/world.h
        ./include/simulation.h
        ./include/potential.h
        ./include/cubicgrid.h
        ./include/openclhelper.h

        ./include/variable.h
        ./include/parameters.h
        ./include/keyvalueparser.h

        ./include/agent.h
        ./include/chargeagent.h
        ./include/fluxagent.h
        ./include/drainagent.h
        ./include/sourceagent.h

        ./include/output.h
        ./include/writer.h
        ./include/checkpointer.h
)

set(RESOURCES
        resources.qrc
)

set(FORMS
)

# INCLUDE
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# FIND
find_boost()
find_opencl()
find_qt()

# TARGET
add_library(${PROJECT_NAME} SHARED EXCLUDE_FROM_ALL ${SOURCES} ${HEADERS} ${RESOURCES} ${FORMS})

# LINK
link_opencl(${PROJECT_NAME})
link_boost(${PROJECT_NAME})
link_qt(${PROJECT_NAME})

# INSTALL
#install(TARGETS ${PROJECT_NAME} DESTINATION bin)
