# ext_drivers target
add_library(ext_drivers STATIC)

target_include_directories(ext_drivers PUBLIC .)

target_sources(ext_drivers PRIVATE mas6181b.c)
target_sources(ext_drivers PRIVATE ds1307.c)
target_sources(ext_drivers PRIVATE hd44780.c)
target_sources(ext_drivers PRIVATE buzzer.c)
target_sources(ext_drivers PRIVATE button.c)
target_sources(ext_drivers PRIVATE rotary_encoder.c)
