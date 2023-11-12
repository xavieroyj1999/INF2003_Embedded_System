add_executable(barcode
        barcode.c
        )

# pull in common dependencies
target_link_libraries(barcode pico_stdlib hardware_adc)
pico_enable_stdio_usb(barcode 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(barcode)