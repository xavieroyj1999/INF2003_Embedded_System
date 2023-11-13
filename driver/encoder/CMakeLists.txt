
if (EXISTS ${PICO_LWIP_CONTRIB_PATH}/apps/ping/ping.c)
    add_executable(encoder
            encoder.c
            ../wifi/ssi.h
            ../../include/global_variables.h
            ${PICO_LWIP_CONTRIB_PATH}/apps/ping/ping.c
    )
    target_include_directories(encoder PRIVATE
        ${PICO_LWIP_CONTRIB_PATH}/apps/ping
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/../.. # for our common lwipopts
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include
        ${CMAKE_CURRENT_SOURCE_DIR}/../wifi
)

target_link_libraries(encoder
        pico_cyw43_arch_lwip_threadsafe_background
        pico_stdlib
        FreeRTOS-Kernel
        FreeRTOS-Kernel-Heap4
        pico_lwip_http
        pico_lwip_iperf
        hardware_adc
)

endif()

pico_add_extra_outputs(encoder)

pico_enable_stdio_usb(encoder 1)

target_include_directories(encoder PUBLIC ${PROJECT_SOURCE_DIR}/include)