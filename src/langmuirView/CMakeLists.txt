project(langmuirView)
cmake_minimum_required(VERSION 2.8)

message(STATUS "Project: ${PROJECT_NAME}")

set(SOURCES
        gridview.cpp
)

set(HEADERS
        ./include/gridview.h
)

set(RESOURCES
)

set(FORMS
)

# must have OpenGL
find_opengl()

if(OPENGL_FOUND)
    # INCLUDE
    include_directories(${langmuirCore_SOURCE_DIR}/include)
    include_directories(${langmuirView_SOURCE_DIR}/include)

    # FIND
    find_boost()
    find_opencl()
    find_qt()

    # TARGET
    add_executable(${PROJECT_NAME} langmuirview.cpp ${SOURCES} ${HEADERS} ${RESOURCES} ${FORMS})

    # LINK
    target_link_libraries(${PROJECT_NAME} langmuirCore)
    link_opencl(${PROJECT_NAME})
    link_opengl(${PROJECT_NAME})
    link_boost(${PROJECT_NAME})
    link_qt(${PROJECT_NAME})

    # INSTALL
    install(TARGETS ${PROJECT_NAME} DESTINATION ${PROJECT_SOURCE_DIR}/bin)
else()
    message(STATUS "No OpenGL : langmuirView disabled")
endif(OPENGL_FOUND)
