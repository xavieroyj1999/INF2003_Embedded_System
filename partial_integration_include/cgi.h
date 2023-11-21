#include "lwip/apps/httpd.h"
#include "pico/cyw43_arch.h"

const char *cgi_synchronise_wheels_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    // I don't need to care about the params
    // gpio_set_irq_enabled_with_callback(LINFRARED_PIN, GPIO_IRQ_EDGE_RISE, false, &interrupt_callback);
    // gpio_set_irq_enabled_with_callback(RINFRARED_PIN, GPIO_IRQ_EDGE_RISE, false, &interrupt_callback);
    printf("Synchronising wheels\n");
    xMessageBufferSend(
        send_sync_duty_cycle_buffer,
        (void *)&duty_cycle_ratio,
        sizeof(duty_cycle_ratio),
        portMAX_DELAY);
    xMessageBufferReceive(
        receive_sync_duty_cycle_buffer,
        (void *)&duty_cycle_ratio,
        sizeof(duty_cycle_ratio),
        portMAX_DELAY);

    // Return HTML back to the server
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
    printf("Loading SHTML\n");
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
    printf("DOES CGI WORKS\n");
    http_set_cgi_handlers(cgi_handlers, 2);
}