add_executable(ideal_mapping
        ideal_mapping.c
        )

# pull in common dependencies
target_link_libraries(ideal_mapping pico_stdlib
        pico_stdio
)
pico_enable_stdio_usb(ideal_mapping 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(ideal_mapping)