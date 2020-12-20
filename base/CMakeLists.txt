#aux_source_directory(. DIR_SRCS)
file(GLOB DIR_SRCS "./*.c" )

add_library (base SHARED ${DIR_SRCS})

target_link_libraries(base pthread yaml protobuf-c ssl crypto)
