add_library(sik-common STATIC err.c messages.c utils.c crc32.c list.c stack.c)
set_target_properties(sik-common PROPERTIES LINKER_LANGUAGE C)
target_include_directories(sik-common PUBLIC ${CMAKE_CURRENT_LIST_DIR})
