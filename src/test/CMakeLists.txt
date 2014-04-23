project(test)
cmake_minimum_required(VERSION 2.8)

message(STATUS "Project: ${PROJECT_NAME}")

# INCLUDE
include_directories(${langmuirCore_SOURCE_DIR}/include)

# FIND
find_boost()
find_opencl()
find_qt()

# TARGET
add_executable(${PROJECT_NAME} EXCLUDE_FROM_ALL test.cpp)

# LINK
target_link_libraries(${PROJECT_NAME} langmuirCore)
link_opencl(${PROJECT_NAME})
link_boost(${PROJECT_NAME})
link_qt(${PROJECT_NAME})
