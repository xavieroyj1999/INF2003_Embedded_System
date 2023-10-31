add_executable(motor
        motor.c
)

target_include_directories(motor PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(motor
        pico_stdlib
        FreeRTOS-Kernel-Heap4
        hardware_pwm
)

pico_add_extra_outputs(motor)

pico_enable_stdio_usb(motor 1)

example_auto_set_url(motor)