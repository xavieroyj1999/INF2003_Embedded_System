#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "message_buffer.h"
#include "task.h"
#include "semphr.h"
#include "hardware/adc.h"
#include "pico/cyw43_arch.h"
#include "lwip/apps/httpd.h"
#include "lwipopts.h"

#include <FreeRTOSConfig.h>
#include <definitions.h>
#include <global_variables.h>
#include <function_prototype.h>
#include <ssi.h>
#include <cgi.h>
#include <initialize_pins.h>
#include <motor_control.h>
#include <ultrasonic.H>
#include <barcode.h>
#include <interrupts_tasks.h>

void webserver_task(__unused void *params)
{
    cyw43_arch_init();
    cyw43_arch_enable_sta_mode();

    // Connect to the WiFI network - loop until connected
    while (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000) != 0);

    httpd_init(); // Initialise web server

    // Configure SSI and CGI handler
    ssi_init();
    cgi_init();

    while (true) vTaskDelay(1000);
}

void main_task()
{
    double left_wheel_duty_cycle = 0;
    double right_wheel_duty_cycle = 0;
    double duty_cycle = 0;
    double duty_cycle_ratio = 1;
    char event_to_perform = '\0';
    enum directionEvent direction = STAY;
    sleep_ms(5000); // Sleep for enough time for user to view print statement
    disable_all_interrupts();
    while (1)
    {
        printf("Choose what do:\n1. choose duty cycle\n2. sync wheel\n3. Straight Path\n4. Right Turn\n5. Left Turn\n6. Barcode\n7. Ultrasonic");
        event_to_perform = getchar();
        switch (event_to_perform)
        {
        case '1': // Choose Duty Cycle
            // Buffer send to choose_duty_cycle_task in motor_control.c
            xMessageBufferSend(
                g_send_choose_duty_cycle_buffer,
                (void *)&duty_cycle_ratio,
                sizeof(duty_cycle_ratio),
                portMAX_DELAY);
            xMessageBufferReceive(
                g_receive_choose_duty_cycle_buffer,
                (void *)&duty_cycle,
                sizeof(duty_cycle),
                portMAX_DELAY);
            break;
        case '2': // Sync Wheel, using interrupts for encoder interrupt ratio calculation
            enable_encoder_interrupts();
            // Buffer send to sync_wheel_task in motor_control.c
            xMessageBufferSend(
                g_send_sync_duty_cycle_buffer,
                (void *)&duty_cycle_ratio,
                sizeof(duty_cycle_ratio),
                0);
            xMessageBufferReceive(
                g_receive_sync_duty_cycle_buffer,
                (void *)&duty_cycle_ratio,
                sizeof(duty_cycle_ratio),
                portMAX_DELAY);
            break;
        case '3': // Move Striaght Line
            enable_encoder_interrupts();
            enable_infrared_interrupts();
            // Semaphore provided to straight_path_task in motor_control.c
            xSemaphoreGive(g_straight_path_task_semaphore);
            xSemaphoreTake(g_main_task_semaphore, portMAX_DELAY);
            break;
        case '4': // Move Straight, Turn Right, Continue Move Straight
            enable_all_interrupts();
            xSemaphoreGive(g_straight_path_task_semaphore);
            // If wall was detected within the next 5 seconds, semaphore would be provided and the vehicle will turn right
            if (xSemaphoreTake(g_wall_semaphore, pdMS_TO_TICKS(5000)) == pdPASS)
            {
                direction = EAST;
                send_buffer_to_wheels(direction);
                gb_wall_detected = false;
            }
            xSemaphoreTake(g_main_task_semaphore, portMAX_DELAY);
            // After turning right, the car will move straight again
            xSemaphoreGive(g_straight_path_task_semaphore);
            xSemaphoreTake(g_main_task_semaphore, portMAX_DELAY);
            break;
        case '5': // Move Straight, Turn Left, Continue Move Straight
            enable_all_interrupts();
            xSemaphoreGive(g_straight_path_task_semaphore);
            // If wall was detected within the next 5 seconds, semaphore would be provided and the vehicle will turn left
            if (xSemaphoreTake(g_wall_semaphore, pdMS_TO_TICKS(5000)) == pdPASS)
            {
                direction = WEST;
                send_buffer_to_wheels(direction);
                gb_wall_detected = false;
            }
            xSemaphoreTake(g_main_task_semaphore, portMAX_DELAY);
            // After turning left, the car will move straight again
            xSemaphoreGive(g_straight_path_task_semaphore);
            xSemaphoreTake(g_main_task_semaphore, portMAX_DELAY);
            break;
        case '6': // Barcode, move car straight while scanning barcode
            xSemaphoreGive(g_straight_path_task_semaphore);
            xSemaphoreGive(g_barcode_task_sempahore);
            xSemaphoreTake(g_barcode_task_complete, portMAX_DELAY); // Wait for barcode task to complete
            xSemaphoreTake(g_main_task_semaphore, portMAX_DELAY);
            break;
        case '7': // Ultrasonic
            enable_ultrasonic_interrupts();
            xSemaphoreGive(g_straight_path_task_semaphore); // Start moving straight
            // If object was detected within the next 5 seconds, semaphore would be provided and the vehicle will u-turn
            if (xSemaphoreTake(g_object_semaphore, pdMS_TO_TICKS(5000)) == pdPASS)
            {
                gpio_set_irq_enabled_with_callback(ECHO_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, false, &interrupt_callback);
                direction = SOUTH;
                send_buffer_to_wheels(direction);
                gb_object_detected = false;
            }
            xSemaphoreTake(g_main_task_semaphore, portMAX_DELAY);
            // After u-turn, the car will move straight again
            xSemaphoreGive(g_straight_path_task_semaphore);
            xSemaphoreTake(g_main_task_semaphore, portMAX_DELAY);
        default:
            break;
        }
        disable_all_interrupts();
    }
}

int main()
{
    stdio_init_all();

    infrared_init();
    encoder_init();
    ultrasonic_init();
    motor_control_init();

    start_tasks();
    return 0;
}