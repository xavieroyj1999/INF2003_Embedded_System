#include "lwip/apps/httpd.h"
#include "pico/cyw43_arch.h"
#include "hardware/adc.h"
#include "stdint.h"  // Include the header for uint32_t
#include "global_variables.h"

// Declare distance_travelled (extern indicates that it's declared elsewhere)
// extern uint32_t distance_travelled;

// SSI tags - tag length limited to 8 bytes by default
const char * ssi_tags[] = {"volt", "temp", "led", "distance"};

u16_t ssi_handler(int iIndex, char *pcInsert, int iInsertLen) {
  size_t printed;
  switch (iIndex) {
  case 0: // volt
    {
      const float voltage = adc_read() * 3.3f / (1 << 12);
      printed = snprintf(pcInsert, iInsertLen, "%f", voltage);
      printf(" Voltage: %f \n", voltage);
    }
    break;
  case 1: // temp
    {
    const float voltage = adc_read() * 3.3f / (1 << 12);
    const float tempC = 27.0f - (voltage - 0.706f) / 0.001721f;
    printed = snprintf(pcInsert, iInsertLen, "%f", tempC);
    printf(" Temp: %f \n", tempC);
    }
    break;
  case 2: // led
    {
      bool led_status = cyw43_arch_gpio_get(CYW43_WL_GPIO_LED_PIN);
      if(led_status == true){
        printed = snprintf(pcInsert, iInsertLen, "ON");
        printf("LED is on \n"); 
      }
      else{
        printed = snprintf(pcInsert, iInsertLen, "OFF");
        printf("LED is off \n");
      }
    }
  case 3: // distance
    {
      printed = snprintf(pcInsert, iInsertLen, "%d", g_distance_travelled);
      printf(" Distance: %d \n", g_distance_travelled);
    }
    break;
  default:
    printed = 0;
    break;
  }

  return (u16_t)printed;
}

// Initialise the SSI handler
void ssi_init() {
  // Initialise ADC (internal pin)
  adc_init();
  adc_set_temp_sensor_enabled(true);
  adc_select_input(4);

  http_set_ssi_handler(ssi_handler, ssi_tags, LWIP_ARRAYSIZE(ssi_tags));
}