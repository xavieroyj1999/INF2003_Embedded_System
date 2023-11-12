#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "FreeRTOS.h"
#include "task.h"

#include <FreeRTOSConfig.h>

#define GENERATE_SOUND_TASK (tskIDLE_PRIORITY + 1UL)

#define TRIG_PIN 6
#define ECHO_PIN 7

#define HIGH 1
#define LOW 0

void init_gpio();
double calculate_distance(uint64_t time_elapsed);
void handle_sound_interrupt(uint gpio, uint32_t events);
void generate_sound_task(void* pvParameters);
void print_distance(double distance);

// Callback function for the repeating timer.
void generate_sound_task(void* pvParameters) {
    while(1) {
        vTaskDelay(1000);
        gpio_put(TRIG_PIN, HIGH);
        vTaskDelay(1);
        gpio_put(TRIG_PIN, LOW);
    }
}

//Configure pins
void init_gpio(){
    gpio_init(TRIG_PIN);
    gpio_set_dir(TRIG_PIN, GPIO_OUT);
    gpio_put(TRIG_PIN, LOW);
    gpio_init(ECHO_PIN);
    gpio_set_dir(ECHO_PIN, GPIO_IN);
}

void handle_sound_interrupt(uint gpio, uint32_t events) {
    static uint64_t start_time = 0;
    uint64_t time_elapsed = 0;
    double distance = 0;
    if (events == GPIO_IRQ_EDGE_FALL)
    {
        time_elapsed = time_us_64() - start_time;
        distance = calculate_distance(time_elapsed);
        if (distance < 5)
        {
            printf("Too close!\n");
        }
        
    }
    else if (events == GPIO_IRQ_EDGE_RISE)
    {
        start_time = time_us_64();
    }
}

double calculate_distance(uint64_t time_elapsed) {
    /* Speed of sound 343 meters per second
    Back and Forth = 343m/2 = 171.5m/second
    1 second = 1000000 microsecond
    17150cm/1000000 microsecond = 0.01715cm/us */

    double distance = (double) time_elapsed * 0.01715;
    printf("Distance: %f cm\n", distance);
    return distance;
}

int main(){
    stdio_init_all();
    init_gpio();

    TaskHandle_t task_generate_sound;
    xTaskCreate(generate_sound_task, "sound thread", configMINIMAL_STACK_SIZE, NULL, GENERATE_SOUND_TASK, &task_generate_sound);

    gpio_set_irq_enabled_with_callback(ECHO_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &handle_sound_interrupt);

    vTaskStartScheduler();
    while(1);
}