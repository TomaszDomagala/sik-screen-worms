add_library(sik-common STATIC err.c messages.c crc32.c utils/utils.c utils/parse.c utils/list.c utils/stack.c)
set_target_properties(sik-common PROPERTIES LINKER_LANGUAGE C)
target_include_directories(sik-common PUBLIC ${CMAKE_CURRENT_LIST_DIR})
