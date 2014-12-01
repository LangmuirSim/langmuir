project(QGLViewer)
cmake_minimum_required(VERSION 2.8)

message(STATUS "Project: ${PROJECT_NAME}")

set(SOURCES
    ./QGLViewer/keyFrameInterpolator.cpp
    ./QGLViewer/frame.cpp
    ./QGLViewer/manipulatedCameraFrame.cpp
    ./QGLViewer/qglviewer.cpp
    ./QGLViewer/constraint.cpp
    ./QGLViewer/quaternion.cpp
    ./QGLViewer/camera.cpp
    ./QGLViewer/vec.cpp
    ./QGLViewer/saveSnapshot.cpp
    ./QGLViewer/manipulatedFrame.cpp
    ./QGLViewer/mouseGrabber.cpp

    ./QGLViewer/VRender/FIGExporter.cpp
    ./QGLViewer/VRender/BackFaceCullingOptimizer.cpp
    ./QGLViewer/VRender/VisibilityOptimizer.cpp
    ./QGLViewer/VRender/BSPSortMethod.cpp
    ./QGLViewer/VRender/ParserGL.cpp
    ./QGLViewer/VRender/gpc.cpp
    ./QGLViewer/VRender/TopologicalSortMethod.cpp
    ./QGLViewer/VRender/Exporter.cpp
    ./QGLViewer/VRender/Vector3.cpp
    ./QGLViewer/VRender/Vector2.cpp
    ./QGLViewer/VRender/PrimitivePositioning.cpp
    ./QGLViewer/VRender/NVector3.cpp
    ./QGLViewer/VRender/EPSExporter.cpp
    ./QGLViewer/VRender/VRender.cpp
    ./QGLViewer/VRender/Primitive.cpp
)

set(HEADERS
    ./QGLViewer/manipulatedFrame.h
    ./QGLViewer/constraint.h
    ./QGLViewer/quaternion.h
    ./QGLViewer/mouseGrabber.h
    ./QGLViewer/qglviewer.h
    ./QGLViewer/config.h
    ./QGLViewer/vec.h
    ./QGLViewer/domUtils.h
    ./QGLViewer/keyFrameInterpolator.h
    ./QGLViewer/manipulatedCameraFrame.h
    ./QGLViewer/frame.h
    ./QGLViewer/camera.h

    ./QGLViewer/VRender/gpc.h
    ./QGLViewer/VRender/Exporter.h
    ./QGLViewer/VRender/SortMethod.h
    ./QGLViewer/VRender/Types.h
    ./QGLViewer/VRender/Vector3.h
    ./QGLViewer/VRender/PrimitivePositioning.h
    ./QGLViewer/VRender/AxisAlignedBox.h
    ./QGLViewer/VRender/Vector2.h
    ./QGLViewer/VRender/NVector3.h
    ./QGLViewer/VRender/ParserGL.h
    ./QGLViewer/VRender/Optimizer.h
    ./QGLViewer/VRender/Primitive.h
    ./QGLViewer/VRender/VRender.h
)

set(RESOURCES
)

set(FORMS
    ./QGLViewer/ImageInterface.ui
    ./QGLViewer/VRenderInterface.ui
)

find_package(Qt5Core)
find_package(Qt5Gui)
find_package(Qt5Xml)
find_package(Qt5OpenGL)
find_package(Qt5Widgets)
find_package(Qt5Concurrent)
find_package(Qt5PrintSupport)
find_package(OpenGL)

if(Qt5Core_FOUND AND OPENGL_FOUND AND NOT USE_QT4)
    # FIND
    find_boost()
    find_opencl()

    # INCLUDE
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
    include_directories(${langmuirCore_SOURCE_DIR}/include)
    include_directories(${CMAKE_CURRENT_BINARY_DIR})
    include_directories(${OPENGL_INCLUDE_DIRS})

    set(CMAKE_AUTOMOC ON)

    qt5_add_resources(RESOURCES ${RESOURCES})
    qt5_wrap_ui(FORMS ${FORMS})

    add_definitions(-DLANGMUIR_USING_QT5)

    # TARGET
    add_library(${PROJECT_NAME} SHARED EXCLUDE_FROM_ALL
        ${SOURCES} ${HEADERS} ${RESOURCES} ${FORMS})

    # LINK
    target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})

    link_opencl(${PROJECT_NAME})
    link_boost(${PROJECT_NAME})

    qt5_use_modules(${PROJECT_NAME} Core)
    qt5_use_modules(${PROJECT_NAME} Gui)
    qt5_use_modules(${PROJECT_NAME} Xml)
    qt5_use_modules(${PROJECT_NAME} OpenGL)
    qt5_use_modules(${PROJECT_NAME} Widgets)
    qt5_use_modules(${PROJECT_NAME} Concurrent)
    qt5_use_modules(${PROJECT_NAME} PrintSupport)
else()
    message(STATUS "Project: ${PROJECT_NAME} Disabled")
    if(Qt5Core_FOUND)
        message(STATUS "qt5 required")
    endif()
endif()
