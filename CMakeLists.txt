cmake_minimum_required (VERSION 3.1)

project (gp C)

# add compile information display
set(CMAKE_VERBOSE_MAKEFILE ON)

# set executable file path
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

include_directories(${PROJECT_SOURCE_DIR}/include)


add_subdirectory(test/thread)
add_subdirectory(test/list)
add_subdirectory(test/threadpool)


add_subdirectory(base)


