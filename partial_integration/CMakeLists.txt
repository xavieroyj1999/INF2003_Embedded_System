set(WIFI_SSID "sakana")
set(WIFI_PASSWORD "88888888")

message("Running makefsdata python script")
execute_process(COMMAND
    py makefsdata.py
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/partial_integration_include
)

add_executable(partial_integration partial_integration.c)

target_compile_definitions(partial_integration PRIVATE
            WIFI_SSID=\"${WIFI_SSID}\"
            WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
            )

# Add required libraries
target_link_libraries(partial_integration
    pico_stdlib
    hardware_i2c
    hardware_pwm
    FreeRTOS-Kernel-Heap4
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_http
    pico_lwip_iperf
    hardware_adc
    m
)

# Create map/bin/hex file etc.
pico_add_extra_outputs(partial_integration)

pico_enable_stdio_usb(partial_integration 1)

target_include_directories(partial_integration PUBLIC ${PROJECT_SOURCE_DIR}/partial_integration_include)