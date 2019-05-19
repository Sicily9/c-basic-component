aux_source_directory(thread DIR_SRCS)
#MESSAGE(STATUS "Src file: ${DIR_SRCS}")

aux_source_directory(list DIR_SRCS)
#MESSAGE(STATUS "Src file: ${DIR_SRCS}")

aux_source_directory(threadpool DIR_SRCS)
#MESSAGE(STATUS "Src file: ${DIR_SRCS}")

add_library (base SHARED ${DIR_SRCS})

target_link_libraries(base pthread)

