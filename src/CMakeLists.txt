add_executable(main main.c)

# Add required libraries
target_link_libraries(main
    pico_stdlib
    hardware_i2c
    hardware_pwm
    FreeRTOS-Kernel-Heap4
    m
)

# Create map/bin/hex file etc.
pico_add_extra_outputs(main)

target_include_directories(main PUBLIC ${PROJECT_SOURCE_DIR}/include)