add_executable(irline
        irline.c
)

target_include_directories(irline PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(irline
        pico_stdlib
        hardware_adc
        FreeRTOS-Kernel-Heap4
)

pico_add_extra_outputs(irline)

pico_enable_stdio_usb(irline 1)

target_include_directories(irline PUBLIC ${PROJECT_SOURCE_DIR}/include)