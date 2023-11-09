add_executable(main main.c)

# Add required libraries
target_link_libraries(main
    pico_stdlib              # for core functionality
    pico_cyw43_arch_none     # we need Wifi to access the GPIO, but we don't need anything else
    FreeRTOS-Kernel-Heap4 
)

# Create map/bin/hex file etc.
pico_add_extra_outputs(main)

target_include_directories(main PUBLIC ${PROJECT_SOURCE_DIR}/include)