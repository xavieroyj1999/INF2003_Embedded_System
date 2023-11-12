add_executable(ultrasonic
        ultrasonic.c
)

target_include_directories(ultrasonic PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(ultrasonic pico_stdlib
        pico_stdio
        hardware_pwm
        FreeRTOS-Kernel-Heap4
)
pico_add_extra_outputs(ultrasonic)

pico_enable_stdio_usb(ultrasonic 1)

target_include_directories(ultrasonic PUBLIC ${PROJECT_SOURCE_DIR}/include)