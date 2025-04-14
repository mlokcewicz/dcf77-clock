# drivers target
add_library(drivers STATIC)

target_include_directories(drivers PUBLIC .)

target_sources(drivers PRIVATE twi.c)
target_sources(drivers PRIVATE exti.c)
target_sources(drivers PRIVATE timer.c)