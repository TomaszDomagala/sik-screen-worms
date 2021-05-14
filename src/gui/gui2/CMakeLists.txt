## Thanks to @danger89 and @Ilothar for updating the gist.
## Set the name and the supported language of the project
#project(sik-worms-gui C)
#
## Set the minimum version of cmake required to build this project
#cmake_minimum_required(VERSION 3.10)


# the `pkg_check_modules` function is created with this call
find_package(PkgConfig REQUIRED)

# these calls create special `PkgConfig::<MODULE>` variables
pkg_check_modules(GTK2 REQUIRED IMPORTED_TARGET gtk+-2.0)

add_executable(screen-worms-gui gui2.c cmd.c net.c read_line.c err.c)

target_link_libraries(screen-worms-gui PUBLIC PkgConfig::GTK2)


## Use the package PkgConfig to detect GTK+ headers/library files
#find_package(PkgConfig REQUIRED)
#pkg_check_modules(GTK2 REQUIRED gtk+-2.0)
#
#add_executable(worms-gui gui2.c cmd.c net.c read_line.c err.c)
#target_link_libraries(worms-gui PRIVATE ${GTK2_LIBRARIES})
#
## Add other flags to the compiler
#target_compile_definitions(worms-gui PRIVATE ${GTK2_CFLAGS})
#target_compile_definitions(worms-gui PRIVATE ${GTK2_CFLAGS_OTHER})
#
## Setup CMake to use GTK+, tell the compiler where to look for headers
## and to the linker where to look for libraries
#target_include_directories(worms-gui PRIVATE ${GTK2_INCLUDE_DIRS})
#target_link_directories(worms-gui PRIVATE ${GTK2_LIBRARY_DIRS})

