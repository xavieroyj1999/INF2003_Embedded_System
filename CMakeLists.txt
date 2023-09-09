cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)


project(INF2003_Embedded_System C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_BOARD pico_w)

# Initialize the SDK
pico_sdk_init()

# Add executable. Default name is the project name
add_executable(main main.c)

# Add required libraries
target_link_libraries(main
    pico_stdlib              # for core functionality
    pico_cyw43_arch_none     # we need Wifi to access the GPIO, but we don't need anything else
)

# Create map/bin/hex file etc.
pico_add_extra_outputs(main)