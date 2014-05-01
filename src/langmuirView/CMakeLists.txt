project(langmuirView)
cmake_minimum_required(VERSION 2.8)

message(STATUS "Project: ${PROJECT_NAME}")

set(SOURCES
    langmuirviewer.cpp
    sceneobject.cpp
    mainwindow.cpp
    corneraxis.cpp
    color.cpp
    axis.cpp
    box.cpp
)

set(HEADERS
    ./include/langmuirviewer.h
    ./include/sceneobject.h
    ./include/mainwindow.h
    ./include/corneraxis.h
    ./include/color.h
    ./include/axis.h
    ./include/box.h
)

set(RESOURCES
    ./icons/icons.qrc
)

set(FORMS
    ./forms/mainwindow.ui
)

# must have OpenGL
find_opengl()

if(OPENGL_FOUND)
    include_directories(${langmuirCore_SOURCE_DIR}/include)
    include_directories(${langmuirView_SOURCE_DIR}/include)
    include_directories(${QGLViewer_SOURCE_DIR}/VRender)
    include_directories(${QGLViewer_SOURCE_DIR})

    # FIND
    find_opencl()
    find_boost()
    find_qt()

    # TARGET
    add_executable(${PROJECT_NAME} EXCLUDE_FROM_ALL langmuirview.cpp ${SOURCES} ${HEADERS} ${RESOURCES} ${FORMS})

    # LINK
    target_link_libraries(${PROJECT_NAME} langmuirCore)
    target_link_libraries(${PROJECT_NAME} QGLViewer)
    link_opencl(${PROJECT_NAME})
    link_opengl(${PROJECT_NAME})
    link_boost(${PROJECT_NAME})
    link_qt(${PROJECT_NAME})
else()
    message(STATUS "No OpenGL : langmuirView disabled")
endif(OPENGL_FOUND)
