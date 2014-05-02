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
