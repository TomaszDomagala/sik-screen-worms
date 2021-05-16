
# the `pkg_check_modules` function is created with this call
find_package(PkgConfig REQUIRED)

# these calls create special `PkgConfig::<MODULE>` variables
pkg_check_modules(GTK2 REQUIRED IMPORTED_TARGET gtk+-2.0)

add_executable(screen-worms-gui gui2.c cmd.c net.c read_line.c err.c)

target_link_libraries(screen-worms-gui PUBLIC PkgConfig::GTK2)
