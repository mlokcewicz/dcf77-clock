# drivers target
add_library(drivers STATIC)

target_include_directories(drivers PUBLIC .)

target_sources(drivers PRIVATE gpio.c)
target_sources(drivers PRIVATE twi.c)
target_sources(drivers PRIVATE timer.c)
target_sources(drivers PRIVATE exti.c)