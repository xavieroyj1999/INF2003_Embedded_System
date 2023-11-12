cmake_minimum_required(VERSION 3.18)

# Pull in SDK (must be before project)
include(lib/pico_sdk_import.cmake)

set(FREERTOS_KERNEL_PATH C:/FreeRTOS-Kernel-main)

include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)


project(INF2003_Embedded_System C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_BOARD pico_w)

# include(lwip_import.cmake) # lwip config files
SET(FREERTOS_CONFIG_FILE_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/configs/FreeRTOS-Kernel" CACHE STRING "Local Config")
include_directories("${FREERTOS_CONFIG_FILE_DIRECTORY}") # FreeRTOS config files

# Initialize the SDK
pico_sdk_init()

add_library(freertos_config INTERFACE)

set(FREERTOS_PORT GCC_ARM_CM4F CACHE STRING "")

target_include_directories(freertos_config SYSTEM
INTERFACE
    include
)

target_compile_definitions(freertos_config
  INTERFACE
    projCOVERAGE_TEST=0
)

# include(lib/FreeRTOS_Kernel_import.cmake)
# add_subdirectory("FreeRTOS-Kernel")

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        )
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wno-maybe-uninitialized)
endif()

# Add executable. Default name is the project name
add_subdirectory(driver)
# add_subdirectory(src)
include_directories(${PROJECT_SOURCE_DIR}/include)