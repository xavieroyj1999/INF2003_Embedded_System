add_executable(barcode
        barcode.c
        )

target_include_directories(barcode PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# pull in common dependencies
target_link_libraries(barcode pico_stdlib
        hardware_adc
        FreeRTOS-Kernel-Heap4)
pico_enable_stdio_usb(barcode 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(barcode)

target_include_directories(barcode PUBLIC ${PROJECT_SOURCE_DIR}/include)