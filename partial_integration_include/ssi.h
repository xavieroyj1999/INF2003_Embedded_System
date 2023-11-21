#include "lwip/apps/httpd.h"
#include "pico/cyw43_arch.h"
#include "hardware/adc.h"
#include "stdint.h" // Include the header for uint32_t

// Declare distance_travelled (extern indicates that it's declared elsewhere)
// extern uint32_t distance_travelled;

// SSI tags - tag length limited to 8 bytes by default
const char *ssi_tags[] = {"volt", "temp", "distance", "barcode"};

u16_t ssi_handler(int iIndex, char *pcInsert, int iInsertLen)
{
    size_t printed;
    switch (iIndex)
    {
    case 3:
    {
        printed = snprintf(pcInsert, iInsertLen, "%c", g_decoded_value);
        printf(" Barcode: %c \n", g_decoded_value);
        break;
    }
    case 4:
    {
        printed = snprintf(pcInsert, iInsertLen, "%d", g_distance_travelled);
        printf(" Distance: %d \n", g_distance_travelled);
        break;
    }
    default:
        printed = 0;
        break;
        return (u16_t)printed;
    }
}

// Initialise the SSI handler
void ssi_init()
{
    http_set_ssi_handler(ssi_handler, ssi_tags, LWIP_ARRAYSIZE(ssi_tags));
}