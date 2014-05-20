project(langmuirView)
cmake_minimum_required(VERSION 2.8)

message(STATUS "Project: ${PROJECT_NAME}")
set(CMAKE_BUILD_TYPE Debug)

set(SOURCES
    langmuirviewer.cpp
    pointdialog.cpp
    colorbutton.cpp
    sceneobject.cpp
    pointcloud.cpp
    mainwindow.cpp
    corneraxis.cpp
    isosurface.cpp
    light.cpp
    color.cpp
    axis.cpp
    grid.cpp
    mesh.cpp
    box.cpp
)

set(HEADERS
    ./include/langmuirviewer.h
    ./include/pointdialog.h
    ./include/colorbutton.h
    ./include/sceneobject.h
    ./include/pointcloud.h
    ./include/mainwindow.h
    ./include/corneraxis.h
    ./include/isosurface.h
    ./include/light.h
    ./include/color.h
    ./include/axis.h
    ./include/grid.h
    ./include/mesh.h
    ./include/box.h
)

set(RESOURCES
    ./icons/icons.qrc
    ./shaders/shaders.qrc
)
file(GLOB_RECURSE ICONS   *.png )
file(GLOB_RECURSE SHADERS *.glsl)

set(FORMS
    ./forms/pointdialog.ui
    ./forms/mainwindow.ui
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
    include_directories(${langmuirCore_SOURCE_DIR}/include)
    include_directories(${langmuirView_SOURCE_DIR}/include)
    include_directories(${QGLViewer_SOURCE_DIR}/QGLViewerVRender)
    include_directories(${QGLViewer_SOURCE_DIR}/QGLViewer)

    include_directories(${OPENGL_INCLUDE_DIRS})
    include_directories(${CMAKE_CURRENT_BINARY_DIR})

    set(CMAKE_AUTOMOC ON)

    qt5_add_resources(RESOURCES ${RESOURCES})
    qt5_wrap_ui(FORMS ${FORMS})

    add_definitions(-DLANGMUIR_USING_QT5)

    # TARGET
    add_executable(${PROJECT_NAME} EXCLUDE_FROM_ALL langmuirview.cpp
        ${SOURCES} ${HEADERS} ${RESOURCES} ${FORMS} ${ICONS} ${SHADERS})

    # LINK
    target_link_libraries(${PROJECT_NAME} langmuirCore)
    target_link_libraries(${PROJECT_NAME} QGLViewer)

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
