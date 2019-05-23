aux_source_directory(common/thread DIR_SRCS)
aux_source_directory(common/list DIR_SRCS)
aux_source_directory(common/dictionary DIR_SRCS)
aux_source_directory(advanced/threadpool DIR_SRCS)

add_library (base SHARED ${DIR_SRCS})

target_link_libraries(base pthread)

