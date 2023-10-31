add_executable(magnometer
        magnometer.c
)

target_include_directories(magnometer PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(magnometer
        pico_stdlib
        FreeRTOS-Kernel-Heap4
        hardware_i2c
        m
)

pico_add_extra_outputs(magnometer)

pico_enable_stdio_usb(magnometer 1)

example_auto_set_url(magnometer)