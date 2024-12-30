# utils target
add_library(libs STATIC)

target_include_directories(libs PUBLIC .)

target_sources(libs PRIVATE dummy.c)