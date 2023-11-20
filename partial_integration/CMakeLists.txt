add_executable(partial_integration partial_integration.c)

# Add required libraries
target_link_libraries(partial_integration
    pico_stdlib
    hardware_i2c
    hardware_pwm
    FreeRTOS-Kernel-Heap4
    m
    hardware_adc
)

# Create map/bin/hex file etc.
pico_add_extra_outputs(partial_integration)

pico_enable_stdio_usb(partial_integration 1)

target_include_directories(partial_integration PUBLIC ${PROJECT_SOURCE_DIR}/partial_integration_include)