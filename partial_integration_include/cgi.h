#include "lwip/apps/httpd.h"
#include "pico/cyw43_arch.h"

// Synchronise wheels using wifi
const char *cgi_synchronise_wheels_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    double duty_cycle_ratio = 1;

    enable_encoder_interrupts();
    xMessageBufferSend(
        g_send_sync_duty_cycle_buffer,
        (void *)&duty_cycle_ratio,
        sizeof(duty_cycle_ratio),
    0);
    xMessageBufferReceive(
        g_receive_sync_duty_cycle_buffer,
        (void *)&duty_cycle_ratio,
        sizeof(duty_cycle_ratio),
        portMAX_DELAY
    );
    disable_all_interrupts();

    return "/index.shtml";
}

// CGI handler to start or stop the car
const char *cgi_start_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    // ONLOAD CHECK IF HTML IS STARTED
    // Check if an request for LED has been made (/led.cgi?led=x)
    // if (strcmp(pcParam[0], "start") == 0)
    // {
    //     // Look at the argument to check if LED is to be turned on (x=1) or off (x=0)
    //     if (strcmp(pcValue[0], "0") == 0)
    //         printf("stop");
    //     else if (strcmp(pcValue[0], "1") == 0)
    //         printf("start");
    // }
    // Send the index page back to the user
    return "/index.shtml";
}

// tCGI Struct
// Fill this with all of the CGI requests and their respective handlers
// This is to prepare for function calls
static const tCGI cgi_handlers[] = {
    // {// Html request for "/led.cgi" triggers cgi_handler
    //  "/led.cgi", cgi_led_handler},
    {
        // Add your new route here
        "/start.cgi",
        cgi_start_handler,
    },
    {
        "/synchronise-wheels.cgi",
        cgi_synchronise_wheels_handler,
    },
};

void cgi_init(void)
{
    http_set_cgi_handlers(cgi_handlers, 2);
}