project(LangmuirChargeTransport)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_BUILD_TYPE Debug)
set(USE_QT4 ON)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

################################################################################
# Library : Boost
macro(find_boost)
    find_package(Boost REQUIRED)
    include_directories(${Boost_INCLUDE_DIRS})
endmacro(find_boost)

macro(link_boost TARGET)
    target_link_libraries(${TARGET} ${Boost_LIBRARIES})
endmacro(link_boost)

################################################################################
# Library : OpenCL
macro(find_opencl)
    set(OPENCL_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/langmuirCore/include
        CACHE PATH "Path to a file")
    find_package(OpenCL)
    if(${OPENCL_FOUND})
        add_definitions(-DLANGMUIR_OPEN_CL)
        include_directories(${OPENCL_INCLUDE_DIRS})
    else()
        message(STATUS "Can not find OpenCL")
    endif(${OPENCL_FOUND})
endmacro(find_opencl)

macro(link_opencl TARGET)
    if(${OPENCL_FOUND})
        target_link_libraries(${TARGET} ${OPENCL_LIBRARIES})
    endif(${OPENCL_FOUND})
endmacro(link_opencl)

################################################################################
# Library : OpenGL
macro(find_opengl)
    find_package(OpenGL)
    include_directories(${OPENGL_INCLUDE_DIRS})
endmacro(find_opengl)

macro(link_opengl TARGET)
    target_link_libraries(${TARGET} ${OPENGL_LIBRARIES})
endmacro(link_opengl)

################################################################################
# Library : QGLViewer
macro(find_qglviewer)
    find_package(QGLViewer)
    include_directories(${QGLVIEWER_INCLUDE_DIR})
endmacro(find_qglviewer)

macro(link_qglviewer TARGET)
    target_link_libraries(${TARGET} ${QGLVIEWER_LIBRARIES})
endmacro(link_qglviewer)

################################################################################
# Library: Qt4
macro(find_qt4)
    message(STATUS "using qt4")

    set(DESIRED_QT_VERSION 4 CACHE STRING "QT version number")

    if(OPENGL_FOUND)
        set(QT_USE_QTOPENGL TRUE)
    endif(OPENGL_FOUND)

    set(QT_USE_QTXML TRUE)

    find_package(Qt REQUIRED)

    include(${QT_USE_FILE})
    add_definitions(${QT_DEFINITIONS})

    include_directories(${CMAKE_CURRENT_BINARY_DIR})

    set(CMAKE_AUTOMOC ON)

    qt4_add_resources(RESOURCES ${RESOURCES})
    qt4_wrap_ui(FORMS ${FORMS})
endmacro(find_qt4)

macro(link_qt4 TARGET)
    target_link_libraries(${TARGET} ${QT_LIBRARIES})
endmacro(link_qt4)

################################################################################
# Library: Qt5
macro(find_qt5)
    message(STATUS "using qt5")

    if(OPENGL_FOUND)
        find_package(Qt5OpenGL REQUIRED)
    endif(OPENGL_FOUND)

    find_package(Qt5Core REQUIRED)
    find_package(Qt5Gui REQUIRED)
    find_package(Qt5Xml REQUIRED)
    find_package(Qt5Widgets REQUIRED)
    find_package(Qt5Concurrent REQUIRED)
    find_package(Qt5PrintSupport REQUIRED)

    message(STATUS "Qt5Core: ${Qt5Core_VERSION_STRING}")

    include_directories(${CMAKE_CURRENT_BINARY_DIR})

    set(CMAKE_AUTOMOC ON)

    qt5_add_resources(RESOURCES ${RESOURCES})
    qt5_wrap_ui(FORMS ${FORMS})

    add_definitions(-DLANGMUIR_USING_QT5)
endmacro(find_qt5)

macro(link_qt5 TARGET)
    if(OPENGL_FOUND)
        qt5_use_modules(${TARGET} OpenGL)
    endif(OPENGL_FOUND)

    qt5_use_modules(${TARGET} Core)
    qt5_use_modules(${TARGET} Gui)
    qt5_use_modules(${TARGET} Xml)
    qt5_use_modules(${TARGET} Widgets)
    qt5_use_modules(${TARGET} Concurrent)
    qt5_use_modules(${TARGET} PrintSupport)
endmacro(link_qt5)

################################################################################
# Library: Qt(4 or 5)
option(USE_QT4 "use qt4 even if qt5 is found" OFF)

macro(find_qt)
    if(USE_QT4)
        find_qt4()
    else()
        find_package(Qt5Core)
        if(Qt5Core_FOUND)
            find_qt5()
        else()
            find_qt4()
        endif(Qt5Core_FOUND)
    endif(USE_QT4)
endmacro(find_qt)

macro(link_qt TARGET)
    if(USE_QT4)
        link_qt4(${TARGET})
    else()
        if(Qt5Core_FOUND)
            link_qt5(${TARGET})
        else()
            link_qt4(${TARGET})
        endif(Qt5Core_FOUND)
    endif(USE_QT4)
endmacro(link_qt)

################################################################################
# projects
add_subdirectory(langmuirCore)
message("")
add_subdirectory(langmuir)
message("")
add_subdirectory(langmuirView)
message("")
add_subdirectory(test)
message("")
