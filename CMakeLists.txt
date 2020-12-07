cmake_minimum_required (VERSION 3.15)
project (gp_C)

include(GoogleTest)
include_directories(${PROJECT_SOURCE_DIR}/include
		${PROJECT_SOURCE_DIR}/googletest/googletest/include
		${PROJECT_SOURCE_DIR}/test)
link_directories(${PROJECT_SOURCE_DIR}/lib ${PROJECT_SOURCE_DIR}/googletest)

function(build_target)
	CMAKE_PARSE_ARGUMENTS(
		ARG "" "T" "SRC;LIBS" ${ARGN}
	)
	add_executable(${ARG_T} ${ARG_SRC})
	target_link_libraries(${ARG_T} ${ARG_LIBS})
endfunction(build_target)

enable_testing() #这个要在项目根目录里开启

option(test "enable test" ON)
set(CMAKE_VERBOSE_MAKEFILE ON) # add compile information display
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)# set executable file path
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
#set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")
add_definitions("-Wall -g -pg")

add_subdirectory(test)
add_subdirectory(googletest)
add_subdirectory(base)

