project(QGLViewer)
cmake_minimum_required(VERSION 2.8)

message(STATUS "Project: ${PROJECT_NAME}")

set(SOURCES
    ./keyFrameInterpolator.cpp
    ./frame.cpp
    ./manipulatedCameraFrame.cpp
    ./qglviewer.cpp
    ./constraint.cpp
    ./quaternion.cpp
    ./camera.cpp
    ./vec.cpp
    ./saveSnapshot.cpp
    ./manipulatedFrame.cpp
    ./mouseGrabber.cpp

    ./VRender/FIGExporter.cpp
    ./VRender/BackFaceCullingOptimizer.cpp
    ./VRender/VisibilityOptimizer.cpp
    ./VRender/BSPSortMethod.cpp
    ./VRender/ParserGL.cpp
    ./VRender/gpc.cpp
    ./VRender/TopologicalSortMethod.cpp
    ./VRender/Exporter.cpp
    ./VRender/Vector3.cpp
    ./VRender/Vector2.cpp
    ./VRender/PrimitivePositioning.cpp
    ./VRender/NVector3.cpp
    ./VRender/EPSExporter.cpp
    ./VRender/VRender.cpp
    ./VRender/Primitive.cpp
)

set(HEADERS
    ./manipulatedFrame.h
    ./constraint.h
    ./quaternion.h
    ./mouseGrabber.h
    ./qglviewer.h
    ./config.h
    ./vec.h
    ./domUtils.h
    ./keyFrameInterpolator.h
    ./manipulatedCameraFrame.h
    ./frame.h
    ./camera.h

    ./VRender/gpc.h
    ./VRender/Exporter.h
    ./VRender/SortMethod.h
    ./VRender/Types.h
    ./VRender/Vector3.h
    ./VRender/PrimitivePositioning.h
    ./VRender/AxisAlignedBox.h
    ./VRender/Vector2.h
    ./VRender/NVector3.h
    ./VRender/ParserGL.h
    ./VRender/Optimizer.h
    ./VRender/Primitive.h
    ./VRender/VRender.h
)

set(RESOURCES
)

set(FORMS
    ./ImageInterface.ui
    ./VRenderInterface.ui
)

# INCLUDE
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# must have OpenGL
find_opengl()

if(OPENGL_FOUND)
    # FIND
    find_qt()

    # TARGET
    add_library(${PROJECT_NAME} SHARED EXCLUDE_FROM_ALL ${SOURCES} ${HEADERS} ${RESOURCES} ${FORMS})

    # LINK
    link_opengl(${PROJECT_NAME})
    link_qt(${PROJECT_NAME})
else()
    message(STATUS "No OpenGL : langmuirView disabled")
endif(OPENGL_FOUND)
