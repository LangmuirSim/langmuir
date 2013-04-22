PROJECT(Langmuir)
SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.2)

SET(COMPILE_TEST FALSE)
SET(COMPILE_LANGMUIR_VIEW TRUE)
SET(COMPILE_LANGMUIR TRUE)

IF(COMPILE_TEST)
    SET(COMPILE_LANGMUIR_VIEW FALSE)
    SET(COMPILE_LANGMUIR FALSE)
ENDIF(COMPILE_TEST)

SET(PROJECT_SOURCES
    simulation.cpp
    openclhelper.cpp
    writer.cpp
    chargeagent.cpp
    drainagent.cpp
    sourceagent.cpp
    rand.cpp
    world.cpp
    cubicgrid.cpp
    fluxagent.cpp
    potential.cpp
    output.cpp
    keyvalueparser.cpp
    checkpointer.cpp
    tolerance.cpp
    pbsgpuparser.cpp
)

SET(PROJECT_HEADERS
    parameters.h
    world.h
    fluxagent.h
    drainagent.h
    agent.h
    chargeagent.h
    cubicgrid.h
    writer.h
    potential.h
    openclhelper.h
    rand.h
    sourceagent.h
    simulation.h
    output.h
    variable.h
    checkpointer.h
    tolerance.h
    pbsgpuparser.h
)

SET(PROJECT_MOC_HEADERS_1
    agent.h
    cubicgrid.h
    writer.h
    openclhelper.h
    potential.h
    simulation.h
    world.h
    rand.h
    output.h
    keyvalueparser.h
    variable.h
    checkpointer.h
    tolerance.h
    pbsgpuparser.h
)

SET(PROJECT_MOC_HEADERS_2
    gridview.h
)

SET(PROJECT_FORMS
)

SET(PROJECT_RESOURCES
    resources.qrc
)

SET(PROJECT_DATA
)

################################################################################

SET(QT_MIN_VERSION 4.0.1)

IF(${COMPILE_LANGMUIR_VIEW})
    SET(QT_USE_QTOPENGL TRUE)
ENDIF(${COMPILE_LANGMUIR_VIEW})

FIND_PACKAGE(Qt4 REQUIRED)

INCLUDE(${QT_USE_FILE})

QT4_WRAP_CPP(
    PROJECT_MOC_HEADERS_PROCESSED_1
    ${PROJECT_MOC_HEADERS_1}
)

QT4_WRAP_CPP(
    PROJECT_MOC_HEADERS_PROCESSED_2
    ${PROJECT_MOC_HEADERS_2}
)

QT4_WRAP_UI(
    PROJECT_FORMS_PROCESSED
    ${PROJECT_FORMS}
)

QT4_ADD_RESOURCES(
    PROJECT_RESOURCES_PROCESSED
    ${PROJECT_RESOURCES}
)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-Wall -O2)

################################################################################

FIND_PACKAGE(Boost REQUIRED)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${BOOST_PYTHON_INCLUDES})

################################################################################

FIND_PACKAGE(OpenCL)

IF(${OPENCL_FOUND})
    ADD_DEFINITIONS(-DLANGMUIR_OPEN_CL)
    INCLUDE_DIRECTORIES(${OPENCL_INCLUDE_DIRS})
ENDIF(${OPENCL_FOUND})

################################################################################

FIND_PACKAGE(OpenGL)

IF(${OPENGL_FOUND})
    INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIRS})
ENDIF(${OPENGL_FOUND})

################################################################################

IF(${COMPILE_TEST})

    MESSAGE("-- Compiling Test")

    ADD_EXECUTABLE(
        test
        test.cpp
        ${PROJECT_SOURCES}
        ${PROJECT_HEADERS}
        ${PROJECT_MOC_HEADERS_PROCESSED_1}
        ${PROJECT_FORMS_PROCESSED}
        ${PROJECT_RESOURCES_PROCESSED}
    )

    TARGET_LINK_LIBRARIES(
        test
        ${Boost_LIBRARIES}
        ${QT_LIBRARIES}
    )

    IF(${OPENCL_FOUND})
        TARGET_LINK_LIBRARIES(
            test
            ${OPENCL_LIBRARIES}
        )
    ENDIF(${OPENCL_FOUND})

INSTALL(TARGETS test DESTINATION ${PROJECT_SOURCE_DIR}/bin)

ENDIF(${COMPILE_TEST})

################################################################################

IF(${COMPILE_LANGMUIR})

    MESSAGE("-- Compiling Langmuir")

    ADD_EXECUTABLE(
        langmuir
        langmuir.cpp
        ${PROJECT_SOURCES}
        ${PROJECT_HEADERS}
        ${PROJECT_MOC_HEADERS_PROCESSED_1}
        ${PROJECT_FORMS_PROCESSED}
        ${PROJECT_RESOURCES_PROCESSED}
    )

    TARGET_LINK_LIBRARIES(
        langmuir
        ${Boost_LIBRARIES}
        ${QT_LIBRARIES}
    )

    IF(${OPENCL_FOUND})
        TARGET_LINK_LIBRARIES(
            langmuir
            ${OPENCL_LIBRARIES}
        )
    ENDIF(${OPENCL_FOUND})

INSTALL(TARGETS langmuir DESTINATION ${PROJECT_SOURCE_DIR}/bin)

ENDIF(${COMPILE_LANGMUIR})

################################################################################

IF(${COMPILE_LANGMUIR_VIEW})

    IF(${OPENGL_FOUND})

        MESSAGE("-- Compiling LangmuirView")

        ADD_EXECUTABLE(
            langmuirView
            langmuirview.cpp
            gridview.cpp
            ${PROJECT_SOURCES}
            ${PROJECT_HEADERS}
            ${PROJECT_MOC_HEADERS_PROCESSED_1}
            ${PROJECT_MOC_HEADERS_PROCESSED_2}
            ${PROJECT_FORMS_PROCESSED}
            ${PROJECT_RESOURCES_PROCESSED}
        )

        TARGET_LINK_LIBRARIES(
            langmuirView
            ${Boost_LIBRARIES}
            ${QT_LIBRARIES}
            ${OPENGL_LIBRARIES}
        )

        IF(${OPENCL_FOUND})
            TARGET_LINK_LIBRARIES(
            langmuirView
            ${OPENCL_LIBRARIES}
            )
        ENDIF(${OPENCL_FOUND})

    ENDIF(${OPENGL_FOUND})

    INSTALL(TARGETS langmuirView DESTINATION ${PROJECT_SOURCE_DIR}/bin)

ENDIF(${COMPILE_LANGMUIR_VIEW})

################################################################################

FIND_PACKAGE(Doxygen)

IF(DOXYGEN_FOUND)

    CONFIGURE_FILE(
        ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY
    )

    ADD_CUSTOM_TARGET(doc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )

    ADD_CUSTOM_TARGET(show
        gnome-www-browser ${CMAKE_SOURCE_DIR}/doc/html/index.html
        DEPENDS doc
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Opening Documentation" VERBATIM
    )

    SET_DIRECTORY_PROPERTIES(
        PROPERTIES
        ADDITIONAL_MAKE_CLEAN_FILES
        ${CMAKE_SOURCE_DIR}/doc
    )

ENDIF(DOXYGEN_FOUND)

################################################################################
