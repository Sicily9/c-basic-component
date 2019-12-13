cmake_minimum_required (VERSION 3.15)

project (gp_C)

include(GoogleTest)
include_directories(${PROJECT_SOURCE_DIR}/include
		${PROJECT_SOURCE_DIR}/googletest/googletest/include)
link_directories(${PROJECT_SOURCE_DIR}/lib ${PROJECT_SOURCE_DIR}/googletest)

option(test "enable test" ON)
set(CMAKE_VERBOSE_MAKEFILE ON) # add compile information display
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)# set executable file path

function(build_target)
	CMAKE_PARSE_ARGUMENTS(
		ARG "" "T" "SRC;LIBS" ${ARGN}
	)
	add_executable(${ARG_T} ${ARG_SRC})
	target_link_libraries(${ARG_T} ${ARG_LIBS})
endfunction(build_target)

add_definitions("-Wall -g -fdiagnostics-color=auto")
enable_testing() #这个要在项目根目录里开启


add_subdirectory(test)
add_subdirectory(googletest)
add_subdirectory(base)

