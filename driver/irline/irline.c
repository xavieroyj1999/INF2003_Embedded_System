#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "FreeRTOS.h"
#include "task.h"
#include "message_buffer.h"

#include <FreeRTOSConfig.h>

#define ADC_CHANNEL 0
#define IR_PIN 26

#define ADC_READ_TASK (tskIDLE_PRIORITY + 1UL)
#define ADC_PRINT_TASK (tskIDLE_PRIORITY + 1UL)
static MessageBufferHandle_t readingBuffer;

#define mbaTASK_MESSAGE_BUFFER_SIZE (60)

void init_IR()
{
    adc_init();
    gpio_init(IR_PIN);
    gpio_set_dir(IR_PIN, GPIO_IN);
}

uint16_t read_IR()
{
    adc_select_input(ADC_CHANNEL);
    uint16_t result = adc_read();
    return result;
}

void read_task(void *pvParameters)
{
    uint16_t reading = 0;
    while (true)
    {
        vTaskDelay(1000);
        reading = read_IR();
        xMessageBufferSend(readingBuffer, (void *)&reading, sizeof(reading), 0);
    }
}

void print_task(void *pvParameters)
{
    uint16_t reading = 0;
    while (true)
    {
        xMessageBufferReceive(readingBuffer, (void *)&reading, sizeof(reading), portMAX_DELAY);
        printf("Reading: %d\n", reading);
    }
}

void start_tasks()
{
    readingBuffer = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);

    TaskHandle_t task_read;
    xTaskCreate(read_task, "temp thread", configMINIMAL_STACK_SIZE, NULL, ADC_READ_TASK, &task_read);

    TaskHandle_t task_print;
    xTaskCreate(print_task, "print thread", configMINIMAL_STACK_SIZE, NULL, ADC_PRINT_TASK, &task_print);

    vTaskStartScheduler();
}

int main()
{
    stdio_init_all();
    init_IR();
    start_tasks();

    while (1)
        ;
    return 0;
}
