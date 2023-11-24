add_executable(maze_solving
        maze_solving.c
        )

# pull in common dependencies
target_link_libraries(maze_solving pico_stdlib
        pico_stdio
)
pico_enable_stdio_usb(maze_solving 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(maze_solving)