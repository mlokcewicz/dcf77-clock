# platform target
add_library(platform STATIC)

target_include_directories(platform PUBLIC .)

aux_source_directory(. PLATFORM_SRC)
target_sources(platform PRIVATE ${PLATFORM_SRC})
