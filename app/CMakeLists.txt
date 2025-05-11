# main app target
aux_source_directory(. MAIN_APP_SRC)

add_executable(${CMAKE_PROJECT_NAME}.elf ${MAIN_APP_SRC})

target_include_directories(${CMAKE_PROJECT_NAME}.elf PRIVATE .)

target_link_libraries(${CMAKE_PROJECT_NAME}.elf 
    libs
    utils 
    platform 
)

# custom targets
add_custom_target(extended_listing ALL DEPENDS ${CMAKE_PROJECT_NAME}.elf COMMAND ${OBJDUMP} -h -S ${CMAKE_PROJECT_NAME}.elf > "${CMAKE_PROJECT_NAME}.lss")
add_custom_target(size ALL DEPENDS ${CMAKE_PROJECT_NAME}.elf COMMAND ${SIZE} --format=avr --mcu=${TARGET_MCU} ${CMAKE_PROJECT_NAME}.elf)
