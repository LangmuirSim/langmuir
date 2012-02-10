project(langmuir)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
cmake_minimum_required(VERSION 2.8.2)

#set(CMAKE_CXX_FLAGS "-j 4")
#set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
set(QT_MIN_VERSION "4.7.0")
set(QT_USE_QTOPENGL TRUE)

set(project_headers_moc_1
      agent.h
      cubicgrid.h
      inputparser.h
      logger.h
      openclhelper.h
      potential.h
      simulation.h
      world.h
      rand.h
      output.h
)

set(project_headers_moc_2
      gridview.h
)

set(project_headers_common
      parameters.h
      world.h
      fluxagent.h
      drainagent.h
      agent.h
      chargeagent.h
      inputparser.h
      cubicgrid.h
      logger.h
      potential.h
      openclhelper.h
      rand.h
      sourceagent.h
      simulation.h
      output.h
)

set(project_sources_common
      simulation.cpp
      openclhelper.cpp
      inputparser.cpp
      logger.cpp
      chargeagent.cpp
      drainagent.cpp
      sourceagent.cpp
      rand.cpp
      world.cpp
      cubicgrid.cpp
      fluxagent.cpp
      potential.cpp
      output.cpp
)

find_package(Boost REQUIRED)
find_package(Qt4 REQUIRED)
find_package(OpenCL REQUIRED)
find_package(OpenGL REQUIRED)

include(${QT_USE_FILE})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${BOOST_PYTHON_INCLUDES})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${OPENCL_INCLUDE_DIRS})

add_definitions(-Wall)
add_definitions(${QT_DEFINITIONS})

#QT4_WRAP_CPP(moc_output_1 ${meta_object_headers_1})
#QT4_WRAP_CPP(moc_output_2 ${meta_object_headers_2})

QT4_WRAP_CPP(moc_output_1 ${project_headers_moc_1})
QT4_WRAP_CPP(moc_output_2 ${project_headers_moc_2})

add_executable(langmuir langmuir.cpp ${project_sources_common} ${project_headers_common} ${moc_output_1})
target_link_libraries(langmuir ${Boost_LIBRARIES})
target_link_libraries(langmuir ${QT_LIBRARIES})
target_link_libraries(langmuir ${OPENCL_LIBRARIES})

include_directories(${OPENGL_INCLUDE_DIRS})
add_executable(langmuirView langmuirview.cpp gridview.cpp ${project_sources_common} ${project_headers_common} ${moc_output_1} ${moc_output_2})
target_link_libraries(langmuirView ${Boost_LIBRARIES})
target_link_libraries(langmuirView ${QT_LIBRARIES})
target_link_libraries(langmuirView ${OPENCL_LIBRARIES})
target_link_libraries(langmuirView ${OPENGL_LIBRARIES})

#add_executable(test test.cpp ${sources} ${moc_output_1})
#target_link_libraries(test ${Boost_LIBRARIES})
#target_link_libraries(test ${QT_LIBRARIES})
#target_link_libraries(test ${OPENCL_LIBRARIES})

