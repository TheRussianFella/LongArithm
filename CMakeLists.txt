cmake_minimum_required(VERSION 2.8)

project(LongArithm)


set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR})

include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/lib")

add_subdirectory (lib)

add_executable (Test test.c)
target_link_libraries (Test lib)
target_link_libraries (Test m)
