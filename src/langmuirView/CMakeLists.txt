project(langmuirView)
cmake_minimum_required(VERSION 2.8)

message(STATUS "Project: ${PROJECT_NAME}")

set(SOURCES
    mainwindow.cpp
)

set(HEADERS
    ./include/mainwindow.h
)

set(RESOURCES
)

set(FORMS
    ./forms/mainwindow.ui
)

# must have OpenGL
find_opengl()
find_qglviewer()

if(OPENGL_FOUND)
    if (QGLVIEWER_FOUND)
        # INCLUDE
        include_directories(${langmuirCore_SOURCE_DIR}/include)
        include_directories(${langmuirView_SOURCE_DIR}/include)

        # FIND
        find_boost()
        find_opencl()
        find_qt()

        # TARGET
        add_executable(${PROJECT_NAME} EXCLUDE_FROM_ALL langmuirview.cpp ${SOURCES} ${HEADERS} ${RESOURCES} ${FORMS})

        # LINK
        target_link_libraries(${PROJECT_NAME} langmuirCore)
        link_qglviewer(${PROJECT_NAME})
        link_opencl(${PROJECT_NAME})
        link_opengl(${PROJECT_NAME})
        link_boost(${PROJECT_NAME})
        link_qt(${PROJECT_NAME})

        # INSTALL
        # install(TARGETS ${PROJECT_NAME} DESTINATION ${PROJECT_SOURCE_DIR}/bin)
    else()
        message(STATUS "No QGLViewer : langmuirView disabled")
    endif(QGLVIEWER_FOUND)
else()
    message(STATUS "No OpenGL : langmuirView disabled")
endif(OPENGL_FOUND)
