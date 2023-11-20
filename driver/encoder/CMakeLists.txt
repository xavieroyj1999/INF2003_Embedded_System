add_executable(encoder
        encoder.c
)

target_include_directories(encoder PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(encoder
        pico_stdlib
        FreeRTOS-Kernel-Heap4
)

pico_add_extra_outputs(encoder)

pico_enable_stdio_usb(encoder 1)

target_include_directories(encoder PUBLIC ${PROJECT_SOURCE_DIR}/include)