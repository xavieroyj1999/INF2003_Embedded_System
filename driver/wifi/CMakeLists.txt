set(WIFI_SSID "sakana")
set(WIFI_PASSWORD "88888888")

message("Running makefsdata python script")
execute_process(COMMAND
    py makefsdata.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

if (EXISTS ${PICO_LWIP_CONTRIB_PATH}/apps/ping/ping.c)
    add_executable(project
            project.c
            ssi.h
            cgi.h
            ../../include/global_variables.h
            ${PICO_LWIP_CONTRIB_PATH}/apps/ping/ping.c
            )
    target_compile_definitions(project PRIVATE
            WIFI_SSID=\"${WIFI_SSID}\"
            WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
            )
    target_include_directories(project PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}
            ${CMAKE_CURRENT_LIST_DIR}/../.. # for our common lwipopts
            ${PICO_LWIP_CONTRIB_PATH}/apps/ping
            ${CMAKE_CURRENT_SOURCE_DIR}/../../include
            ${CMAKE_CURRENT_SOURCE_DIR}/../driver/encoder
            )
    target_link_libraries(project
            pico_cyw43_arch_lwip_threadsafe_background
            pico_lwip_http
            pico_stdlib
            pico_lwip_iperf
            hardware_adc
            FreeRTOS-Kernel
            FreeRTOS-Kernel-Heap4 # FreeRTOS kernel and dynamic heap
            )
    pico_enable_stdio_usb(project 1)
    pico_add_extra_outputs(project)
endif()