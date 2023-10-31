#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"

#define PRINT_TASK (tskIDLE_PRIORITY + 1UL)

//General Definations
#define CONVERT_TO_MILLISECONDS 1000
#define DEBOUNCE_TIME_MS 50

// Wheel Encoder Configs
#define ONE_INTERRUPT_CM 1

// Left Wheel Encoder Pins
#define LEFT_ENCODER_PIN 14

// Right Wheel Encoder Pins
#define RIGHT_ENCODER_PIN 15

// Variables to track speed of each wheel based off interrupts
uint8_t left_encoder_interrupts = 0;
uint8_t right_encoder_interrupts = 0;
uint32_t distance_travelled = 0;

// Initialise pins
void init_encoder() {
    gpio_init(LEFT_ENCODER_PIN);
    gpio_set_dir(LEFT_ENCODER_PIN, GPIO_IN);
    gpio_init(RIGHT_ENCODER_PIN);
    gpio_set_dir(RIGHT_ENCODER_PIN, GPIO_IN);
}

/**
 * @brief Callback function for the left and right wheel encoder interrupt.
 *
 * These functions are called whenever an interrupt event is triggered by the
 * wheel encoders. They update the distance traveled by the robot and
 * keep track of the number of interrupts.
 */
void left_encoder_callback(uint gpio, uint32_t events) {
    static uint32_t last_time = 0;
    uint32_t current_time = time_us_32();
    if (current_time - last_time < DEBOUNCE_TIME_MS * CONVERT_TO_MILLISECONDS) {
        return;
    }
    distance_travelled += ONE_INTERRUPT_CM;
    last_time = current_time;
    left_encoder_interrupts++;
}

void right_encoder_callback(uint gpio, uint32_t events) {
    static uint32_t last_time = 0;
    uint32_t current_time = time_us_32();
    if (current_time - last_time < DEBOUNCE_TIME_MS * CONVERT_TO_MILLISECONDS) {
        return;
    }
    distance_travelled += ONE_INTERRUPT_CM;
    last_time = current_time;
    right_encoder_interrupts++;
}

void performance_track_task(void* pvParameters) {
    while(1) {
        vTaskDelay(1000);
        printf("Distance: %d cm\n", distance_travelled);
        printf("Left Speed: %d interrupts/second\n", left_encoder_interrupts);
        printf("Right Speed: %d interrupts/second\n", right_encoder_interrupts);
        left_encoder_interrupts = 0;
        right_encoder_interrupts = 0;
    }
}

void start_tasks() {
    TaskHandle_t task_performance;
    xTaskCreate(performance_tracking_task, "track performance thread", configMINIMAL_STACK_SIZE, NULL, PERFORMANCE_TRACK_TASK, &task_performance);

    vTaskStartScheduler();
}

int main() {
    stdio_init_all();
    init_encoder();

    gpio_set_irq_enabled_with_callback(LEFT_ENCODER_PIN, GPIO_IRQ_EDGE_RISE, true, &left_encoder_callback);
    gpio_set_irq_enabled_with_callback(RIGHT_ENCODER_PIN, GPIO_IRQ_EDGE_RISE, true, &right_encoder_callback);

    start_tasks();

    return 0;
}
