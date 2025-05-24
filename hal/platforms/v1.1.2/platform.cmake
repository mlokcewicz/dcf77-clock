# platform targets - will be included in hal/CMakeLists.txt

add_subdirectory(platforms/${HW_VERSION})

# Platform specific defines
add_definitions(-DEXTI_USE_PCINT0_ISR=1)
add_definitions(-DEXTI_USE_PCINT1_ISR=1)

add_definitions(-DTIMER_USE_TIMER0=1)
add_definitions(-DTIMER_USE_TIMER2=1)

add_definitions(-DTIMER_USE_TIMER0_COMPA_ISR=1)

add_definitions(-DTWI_USE_FIXED_100KHZ=1)

add_definitions(-DUSART_USE_FIXED_BAUDRATE=1)
add_definitions(-DUSART_FIXED_BAUDRATE_DOUBLE_SPEED=1)
add_definitions(-DUSART_FIXED_BAUDRATE=9600)

add_definitions(-DROTARY_ENCODER_USE_POLLING=1)
add_definitions(-DBUTTON_USE_POLLING=1)

add_subdirectory(drivers)
add_subdirectory(ext_drivers)
add_subdirectory(startup)