# hardware configuration
set(TARGET_MCU "atmega88p")
set(TARGET_CLOCK "1000000UL")
 
# toolchain configuration
set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_SYSTEM_NAME Generic)

set(CMAKE_C_COMPILER    ${TOOLCHAIN_PATH}avr-gcc.exe)
set(AS                  ${TOOLCHAIN_PATH}avr-as.exe)
set(AR                  ${TOOLCHAIN_PATH}avr-ar.exe)
set(OBJCOPY             ${TOOLCHAIN_PATH}avr-objcopy.exe)
set(OBJDUMP             ${TOOLCHAIN_PATH}avr-objdump.exe)
set(SIZE                ${TOOLCHAIN_PATH}avr-size.exe)
set(NM                  ${TOOLCHAIN_PATH}av-nm.exe)

set(CMAKE_C_USE_RESPONSE_FILE_FOR_OBJECTS      ON)
set(CMAKE_C_USE_RESPONSE_FILE_FOR_INCLUDES     ON)
set(CMAKE_C_USE_RESPONSE_FILE_FOR_LIBRARIES    ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS        	   ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(WARNING_FLAGS 
    -Wall 
    -Wcomment 
    -Wextra 
    -Wno-error=cpp 
    -Wformat-security 
    -Wfloat-equal 
    -Wshadow 
    -Wpointer-arith)

set(CORE_FLAGS 
    -mmcu=${TARGET_MCU} 
    -DF_CPU=${TARGET_CLOCK})

set(C_FLAGS 
    -fpack-struct 
    -fshort-enums 
    -ffunction-sections 
    -fdata-sections 
    -funsigned-char 
    -funsigned-bitfields) 

set(ASM_FLAGS 
    -x 
    assembler-with-cpp 
    -gstabs 
    -nasm)

set(LINKER_FLAGS
    LINKER:--gc-sections
    LINKER:-Map=map_file.map 
    -mmcu=${TARGET_MCU})

set(RELEASE_FLAGS 
    -Os)

add_compile_options(
    "$<$<COMPILE_LANGUAGE:ASM>:${CORE_FLAGS};${ASM_FLAGS}>"
    "$<$<COMPILE_LANGUAGE:C>:${CORE_FLAGS};${C_FLAGS};${WARNING_FLAGS}>"
    "$<$<COMPILE_LANGUAGE:CXX>:${CORE_FLAGS};${CXX_FLAGS};${WARNING_FLAGS}>")

add_compile_options(
    "$<$<CONFIG:DEBUG>:${DEBUG_FLAGS}>"
    "$<$<CONFIG:RELEASE>:${RELEASE_FLAGS}>")

add_link_options(
    ${CORE_FLAGS} 
    ${LINKER_FLAGS})
