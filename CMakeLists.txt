cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)


project(INF2003_Embedded_System C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_BOARD pico_w)

# Initialize the SDK
pico_sdk_init()

include(FreeRTOS_Kernel_import.cmake)

# Add executable. Default name is the project name
add_subdirectory(driver)
add_subdirectory(src)
add_subdirectory(partial_integration)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/partial_integration_include)