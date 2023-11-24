// Copyright (c) 2020-2022 Cesanta Software Limited
// All rights reserved

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

// #include "lwip/ip4_addr.h"

#include "FreeRTOS.h"
#include "task.h"
#include "lwip/apps/httpd.h"

#include "lwipopts.h"
#include "ssi.h"
#include "cgi.h"


#define TEST_TASK_PRIORITY				( tskIDLE_PRIORITY + 1UL )
#define TEST_TASK_STACK_SIZE			(( configSTACK_DEPTH_TYPE ) 2048)

// static struct mg_mgr mgr;
// static const char *s_listening_address = "http://0.0.0.0:80";

void main_task(__unused void *params) {
    cyw43_arch_init();

    cyw43_arch_enable_sta_mode();

    // Connect to the WiFI network - loop until connected
    while(cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000) != 0){
        printf("Attempting to connect...\n");
    }
    // Print a success message once connected
    printf("Connected! \n");
    
    // Initialise web server
    httpd_init();
    printf("Http server initialised\n");

    // Configure SSI and CGI handler
    ssi_init(); 
    printf("SSI Handler initialised\n");
    cgi_init();
    printf("CGI Handler initialised\n");
    
    // Infinite loop
    while(1);
}

void vLaunch( void) {
    TaskHandle_t task;
    xTaskCreate(main_task, "TestMainThread", TEST_TASK_STACK_SIZE, NULL, TEST_TASK_PRIORITY, &task);
    vTaskStartScheduler();
}

int main( void )
{
    stdio_init_all();

    vLaunch();

    return 0;
}