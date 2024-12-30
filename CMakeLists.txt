cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

# toolchain configuration
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/hal/platforms/${HW_VERSION}/toolchain.cmake)

# project configuration
project("embedded_project_name" C ASM)

# Application modules
add_subdirectory(app)
add_subdirectory(middlewares)
add_subdirectory(external)
add_subdirectory(hal)