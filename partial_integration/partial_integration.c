#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "message_buffer.h"
#include "task.h"
#include "math.h"
#include "semphr.h"
#include "hardware/adc.h"
#include "pico/cyw43_arch.h"
#include "lwip/apps/httpd.h"
#include "lwipopts.h"

#include <global_variables.h>
#include <ssi.h>
#include <cgi.h>
#include <FreeRTOSConfig.h>
#include <declarations.h>
#include <function_prototype.h>
#include <ultrasonic.H>
#include <barcode.h>
#include <initialize.h>
#include <magnometer.h>

#define WEB_TASK_PRIORITY (tskIDLE_PRIORITY + 1UL)
#define WEB_TASK_STACK_SIZE ((configSTACK_DEPTH_TYPE)2048)

static MessageBufferHandle_t send_choose_duty_cycle_buffer;
static MessageBufferHandle_t receive_choose_duty_cycle_buffer;

static MessageBufferHandle_t send_sync_duty_cycle_buffer;
static MessageBufferHandle_t receive_sync_duty_cycle_buffer;

static MessageBufferHandle_t g_leftWheelBuffer;
static MessageBufferHandle_t g_rightWheelBuffer;

static SemaphoreHandle_t left_wheel_task_complete;
static SemaphoreHandle_t right_wheel_task_complete;
static SemaphoreHandle_t barcode_task_sempahore;
static SemaphoreHandle_t main_task_semaphore;
static SemaphoreHandle_t magnetometer_task;
static SemaphoreHandle_t straight_path_task_semaphore;
static SemaphoreHandle_t object_semaphore;
static SemaphoreHandle_t wall_semaphore;

void webserver_task(__unused void *params)
{
    cyw43_arch_init();

    cyw43_arch_enable_sta_mode();

    // Connect to the WiFI network - loop until connected
    while (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000) != 0)
    {
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
    while (true)
    {
        vTaskDelay(1000);
    };
}

void interrupt_callback(uint gpio, uint32_t events)
{
    // Right Encoder Distance Counter
    uint32_t current_time = time_us_32();
    if (events & GPIO_IRQ_EDGE_RISE && gpio == RENCODER_PIN)
    {
        static uint32_t rencoder_last_time = 0;
        if (current_time - rencoder_last_time > DEBOUNCE_TIME)
        {
            rencoder_last_time = current_time;
            g_right_encoder_interrupts++;
        }
    }

    // Left Encoder Distance Counter
    if (events & GPIO_IRQ_EDGE_RISE && gpio == LENCODER_PIN)
    {
        static uint32_t lencoder_last_time = 0;
        if (current_time - lencoder_last_time > DEBOUNCE_TIME)
        {
            lencoder_last_time = current_time;
            g_left_encoder_interrupts++;
        }
    }

    // Left Infrared Sensor to readjust degree upon hitting a wall
    if (events & GPIO_IRQ_EDGE_RISE && gpio == LINFRARED_PIN)
    {
        static uint32_t linfrared_last_time = 0;
        if (current_time - linfrared_last_time > DEBOUNCE_TIME)
        {
            linfrared_last_time = current_time;
            gpio_set_irq_enabled_with_callback(RENCODER_PIN, GPIO_IRQ_EDGE_RISE, false, &interrupt_callback);\
            right_wheel_backward();
            while (gpio_get(LINFRARED_PIN) == HIGH);
            g_initial_degree = magneto_read();
            generateDegreeThresholds();
            right_wheel_forward();
            gpio_set_irq_enabled_with_callback(RENCODER_PIN, GPIO_IRQ_EDGE_RISE, true, &interrupt_callback);
        }
    }

    // Right Infrared Sensor to readjust degree upon hitting a wall
    if (events & GPIO_IRQ_EDGE_RISE && gpio == RINFRARED_PIN)
    {
        static uint32_t rinfrared_last_time = 0;
        if (current_time - rinfrared_last_time > DEBOUNCE_TIME)
        {
            rinfrared_last_time = current_time;
            gpio_set_irq_enabled_with_callback(LENCODER_PIN, GPIO_IRQ_EDGE_RISE, true, &interrupt_callback);
            left_wheel_backward();
            while (gpio_get(RINFRARED_PIN) == HIGH);
            g_initial_degree = magneto_read();
            generateDegreeThresholds();
            left_wheel_forward();
            gpio_set_irq_enabled_with_callback(LENCODER_PIN, GPIO_IRQ_EDGE_RISE, true, &interrupt_callback);
        }
    }

    // Scanned front wall
    if (events & GPIO_IRQ_EDGE_RISE && gpio == BINFRARED_D0_PIN)
    {
        static uint32_t bdinfrared_last_time = 0;
        if (current_time - bdinfrared_last_time > DEBOUNCE_TIME)
        {
            bdinfrared_last_time = current_time;
            g_wall_detected = true;
            gpio_set_irq_enabled_with_callback(BINFRARED_D0_PIN, GPIO_IRQ_EDGE_RISE, false, &interrupt_callback);
            left_wheel_backward();
            right_wheel_backward();
            while (gpio_get(BINFRARED_D0_PIN) == HIGH);
            left_wheel_stop();
            right_wheel_stop();
            xSemaphoreGive(wall_semaphore);
        }
    }

    if (gpio == ECHO_PIN)
    {
        static uint64_t start_time = 0;
        uint64_t time_elapsed = 0;
        double distance = 0;
        if (events == GPIO_IRQ_EDGE_FALL)
        {
            time_elapsed = time_us_64() - start_time;
            distance = calculate_distance(time_elapsed);
            if (distance < 5)
            {
                xSemaphoreGive(object_semaphore);
                g_object_detected = true;
            }
        }
        else if (events == GPIO_IRQ_EDGE_RISE)
        {
            start_time = time_us_64();
        }
    }
}

// start of motor.c
void left_wheel_forward()
{
    gpio_put(INB1_PIN, HIGH);
    gpio_put(INB2_PIN, LOW);
}

void right_wheel_forward()
{
    gpio_put(INA1_PIN, HIGH);
    gpio_put(INA2_PIN, LOW);
}

void left_wheel_backward()
{
    gpio_put(INB1_PIN, LOW);
    gpio_put(INB2_PIN, HIGH);
}

void right_wheel_backward()
{
    gpio_put(INA1_PIN, LOW);
    gpio_put(INA2_PIN, HIGH);
}

void left_wheel_stop()
{
    gpio_put(INB1_PIN, LOW);
    gpio_put(INB2_PIN, LOW);
}

void right_wheel_stop()
{
    gpio_put(INA1_PIN, LOW);
    gpio_put(INA2_PIN, LOW);
}

void set_wheel_duty_cycle(double duty_cycle_ratio)
{
    if (duty_cycle_ratio > 1)
    {
        pwm_set_chan_level(GP5_SLICE, PWM_CHAN_B, PWM_FREQ);
        pwm_set_chan_level(GP0_SLICE, PWM_CHAN_A, (int)(PWM_FREQ / duty_cycle_ratio));
    }
    else
    {
        pwm_set_chan_level(GP5_SLICE, PWM_CHAN_B, (int)(PWM_FREQ * duty_cycle_ratio));
        pwm_set_chan_level(GP0_SLICE, PWM_CHAN_A, PWM_FREQ);
    }
}

void sync_wheel_task(void *pvParameters)
{
    double duty_cycle = 0;
    while (true)
    {
        xMessageBufferReceive(
            send_sync_duty_cycle_buffer,
            (void *)&duty_cycle,
            sizeof(duty_cycle),
            portMAX_DELAY);
        printf("Syncing wheel with duty cycle: %f\n", duty_cycle);
        g_current_left_wheel_count = g_left_encoder_interrupts;
        g_current_right_wheel_count = g_right_encoder_interrupts;

        pwm_set_chan_level(GP5_SLICE, PWM_CHAN_B, (int)(PWM_FREQ * duty_cycle));
        pwm_set_chan_level(GP0_SLICE, PWM_CHAN_A, (int)(PWM_FREQ * duty_cycle));

        left_wheel_forward();
        right_wheel_forward();
        vTaskDelay(pdMS_TO_TICKS(10000));
        left_wheel_stop();
        right_wheel_stop();

        uint16_t left_wheel_travelled = g_left_encoder_interrupts - g_current_left_wheel_count;
        uint16_t right_wheel_travelled = g_right_encoder_interrupts - g_current_right_wheel_count;
        double duty_cycle_ratio = (double)left_wheel_travelled / right_wheel_travelled;

        set_wheel_duty_cycle(duty_cycle_ratio);

        xMessageBufferSend(
            receive_sync_duty_cycle_buffer,
            (void *)&duty_cycle_ratio,
            sizeof(duty_cycle_ratio),
            0);
    }
}

void choose_duty_cycle_task(void *pvParameters)
{
    char duty_cycle_selection = '\0';
    double duty_cycle_ratio = 1;

    while (true)
    {
        xMessageBufferReceive(
            send_choose_duty_cycle_buffer,
            (void *)&duty_cycle_ratio,
            sizeof(duty_cycle_ratio),
            portMAX_DELAY);

        printf("Choose duty cycle (0: 0%%, 1: 25%%, 2: 50%%, 3: 75%%, 4: 100%%): ");
        while (true)
        {
            duty_cycle_selection = getchar();
            if (duty_cycle_selection == '0' || duty_cycle_selection == '1' || duty_cycle_selection == '2' || duty_cycle_selection == '3' || duty_cycle_selection == '4')
                break;
            printf("Invalid duty cycle\n");
        }

        double duty_cycle = 0;
        switch (duty_cycle_selection)
        {
        case '0':
            duty_cycle = 0;
            break;
        case '1':
            duty_cycle = 0.25;
            break;
        case '2':
            duty_cycle = 0.5;
            break;
        case '3':
            duty_cycle = 0.75;
            break;
        case '4':
            duty_cycle = 1;
            break;
        }

        xMessageBufferSend(receive_choose_duty_cycle_buffer, &duty_cycle, sizeof(duty_cycle), 0);
    }
}

void straight_path_task(void *pvParameters)
{
    u_int16_t current_left_wheel_count = 0;
    u_int16_t current_right_wheel_count = 0;
    u_int16_t count = 0;
    while (true)
    {
        xSemaphoreTake(straight_path_task_semaphore, portMAX_DELAY);
        current_left_wheel_count = g_left_encoder_interrupts;
        current_right_wheel_count = g_right_encoder_interrupts;
        left_wheel_forward();
        right_wheel_forward();
        while (count <= 100)
        {
            if (g_object_detected || g_wall_detected) {
                left_wheel_stop();
                right_wheel_stop();
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(50));
            count++;
        }
        count = 0;
        left_wheel_stop();
        right_wheel_stop();
        printf("Left wheel travelled: %d\n", g_left_encoder_interrupts - current_left_wheel_count);
        printf("Right wheel travelled: %d\n", g_right_encoder_interrupts - current_right_wheel_count);
        xSemaphoreGive(main_task_semaphore);
    }
}

void left_wheel_task(void *pvParameters)
{
    enum directionEvent direction = STAY;
    u_int16_t current_left_wheel_count = 0;
    while (true)
    {
        xMessageBufferReceive(
            g_leftWheelBuffer,
            (void *)&direction,
            sizeof(direction),
            portMAX_DELAY);
        current_left_wheel_count = g_left_encoder_interrupts;
        if (direction == NORTH || direction == EAST)
        {
            while (true)
            {
                left_wheel_forward();
                vTaskDelay(50);
                if (g_left_encoder_interrupts - current_left_wheel_count > 5)
                    break;
            }
        }
        else if (direction == WEST)
        {
            while (true)
            {
                left_wheel_backward();
                vTaskDelay(50);
                if (g_left_encoder_interrupts - current_left_wheel_count > 5)
                    break;
            }
        }
        else if (direction == SOUTH)
        {
            while (true)
            {
                left_wheel_backward();
                vTaskDelay(50);
                if (g_left_encoder_interrupts - current_left_wheel_count > 10)
                    break;
            }
        }
        else if (direction == STAY)
        {
            left_wheel_stop();
        }
        left_wheel_stop();
        xSemaphoreGive(left_wheel_task_complete);
    }
}

void right_wheel_task(void *pvParameters)
{
    enum directionEvent direction = STAY;
    u_int16_t current_right_wheel_count = 0;
    while (true)
    {
        xMessageBufferReceive(
            g_rightWheelBuffer,
            (void *)&direction,
            sizeof(direction),
            portMAX_DELAY);
        current_right_wheel_count = g_right_encoder_interrupts;
        if (direction == NORTH || direction == WEST)
        {
            while (true)
            {
                right_wheel_forward();
                vTaskDelay(50);
                if (g_right_encoder_interrupts - current_right_wheel_count > 5)
                    break;
            }
        }
        else if (direction == EAST)
        {
            while (true)
            {
                right_wheel_backward();
                vTaskDelay(50);
                if (g_right_encoder_interrupts - current_right_wheel_count > 5)
                    break;
            }
        }
        else if (direction == SOUTH)
        {
            while (true)
            {
                right_wheel_forward();
                vTaskDelay(50);
                if (g_right_encoder_interrupts - current_right_wheel_count > 10)
                    break;
            }
        }
        else if (direction == STAY)
        {
            right_wheel_stop();
        }
        right_wheel_stop();
        xSemaphoreGive(right_wheel_task_complete);
    }
}

void snap_threshold_task()
{
    while (true)
    {
        xSemaphoreTake(magnetometer_task, portMAX_DELAY);
        float current_degree = magneto_read();
        float degree_diffrence = findNearestDegreeDifference(current_degree);
        if (degree_diffrence > 10)
        {
            while (degree_diffrence > 10)
            {
                current_degree = magneto_read();
                degree_diffrence = findNearestDegreeDifference(current_degree);
                vTaskDelay(10);
                left_wheel_backward();
            }
        }
        else if (degree_diffrence < -10)
        {
            while (degree_diffrence < -10)
            {
                current_degree = magneto_read();
                degree_diffrence = findNearestDegreeDifference(current_degree);
                vTaskDelay(10);
                right_wheel_backward();
            }
        }
        left_wheel_stop();
        right_wheel_stop();
        xSemaphoreGive(main_task_semaphore);
    }
}

void barcode_task(void *pvParameters)
{
    bool start_count = false;
    bool last_color_black = true;
    uint8_t odd_bit_index = 0;
    uint8_t even_bit_index = 0;
    uint16_t black_count = 0;
    uint16_t white_count = 0;
    uint16_t odd_count[5] = {0, 0, 0, 0, 0};
    uint16_t even_count[4] = {0, 0, 0, 0};
    uint32_t adc_value = 0;
    uint8_t count = 0;
    while (true)
    {
        xSemaphoreTake(barcode_task_sempahore, portMAX_DELAY);
        while (count < 3)
        {
            adc_value = adc_read();
            if (odd_bit_index == 0 && even_bit_index == 0 && adc_value > COLOR_THRESHOLD)
            {
                start_count = true;
            }

            if (start_count)
            {
                if (adc_value >= COLOR_THRESHOLD)
                {
                    black_count++;
                }
                else if (adc_value < COLOR_THRESHOLD)
                {
                    white_count++;
                }

                if (adc_value >= COLOR_THRESHOLD && !last_color_black && start_count)
                {
                    even_count[even_bit_index] = white_count;
                    even_bit_index++;
                    black_count = 0;
                    last_color_black = true;
                }
                else if (adc_value < COLOR_THRESHOLD && last_color_black && start_count)
                {
                    odd_count[odd_bit_index] = black_count;
                    odd_bit_index++;
                    white_count = 0;
                    last_color_black = false;
                }
            }

            // If we have 9 bits, reset everything and decode the value
            if ((odd_bit_index + even_bit_index) >= 9)
            {
                uint8_t odd_bit = determine_odd_bit(odd_count);
                uint8_t even_bit = determine_even_bit(even_count);
                char decoded_value = code_39_decoder(odd_bit, even_bit);
                printf("Decoded Value: %c\n", decoded_value);
                odd_bit_index = even_bit_index = 0;
                black_count = white_count = 0;
                start_count = false;
                last_color_black = true;
                determine_char(decoded_value);
                count++;
            }
            vTaskDelay(pdMS_TO_TICKS(1));
        }
        count = 0;
    }
}

void generate_sound_task(void *pvParameters)
{
    while (true)
    {
        vTaskDelay(1000);
        gpio_put(TRIG_PIN, HIGH);
        vTaskDelay(1);
        gpio_put(TRIG_PIN, LOW);
    }
}
// end of motor.c

// Start of init_task.c
#define MAIN_TASK (tskIDLE_PRIORITY + 1UL)

void main_task()
{
    double left_wheel_duty_cycle = 0;
    double right_wheel_duty_cycle = 0;
    double duty_cycle = 0;
    double duty_cycle_ratio = 1;
    char event_to_perform = '\0';
    enum directionEvent direction = STAY;
    g_initial_degree = magneto_read();
    sleep_ms(5000);
    while (1)
    {
        printf("Choose what do:\n1. choose duty cycle\n2. sync wheel\n3. Straight Path\n4. Right Turn\n5. Left Turn\n6. Barcode\n7. Ultrasonic");
        event_to_perform = getchar();

        gpio_set_irq_enabled_with_callback(LINFRARED_PIN, GPIO_IRQ_EDGE_RISE, true, &interrupt_callback);
        gpio_set_irq_enabled_with_callback(RINFRARED_PIN, GPIO_IRQ_EDGE_RISE, true, &interrupt_callback);
        gpio_set_irq_enabled_with_callback(LENCODER_PIN, GPIO_IRQ_EDGE_RISE, true, &interrupt_callback);
        gpio_set_irq_enabled_with_callback(RENCODER_PIN, GPIO_IRQ_EDGE_RISE, true, &interrupt_callback);

        switch (event_to_perform)
        {
        case '1': // Choose Duty Cycle
            xMessageBufferSend(
                send_choose_duty_cycle_buffer,
                (void *)&duty_cycle_ratio,
                sizeof(duty_cycle_ratio),
                portMAX_DELAY);
            xMessageBufferReceive(
                receive_choose_duty_cycle_buffer,
                (void *)&duty_cycle,
                sizeof(duty_cycle),
                portMAX_DELAY);
            printf("Duty cycle: %f\n", duty_cycle);
            break;
        case '2': // Sync Wheel
            gpio_set_irq_enabled_with_callback(LINFRARED_PIN, GPIO_IRQ_EDGE_RISE, false, &interrupt_callback);
            gpio_set_irq_enabled_with_callback(RINFRARED_PIN, GPIO_IRQ_EDGE_RISE, false, &interrupt_callback);
            xMessageBufferSend(
                send_sync_duty_cycle_buffer,
                (void *)&duty_cycle_ratio,
                sizeof(duty_cycle_ratio),
                0);
            xMessageBufferReceive(
                receive_sync_duty_cycle_buffer,
                (void *)&duty_cycle_ratio,
                sizeof(duty_cycle_ratio),
                portMAX_DELAY);
            printf("Duty cycle ratio: %f\n", duty_cycle_ratio);
            break;
        case '3': // Move Striaght Line
            xSemaphoreGive(straight_path_task_semaphore);
            xSemaphoreTake(main_task_semaphore, portMAX_DELAY);
            break;
        case '4': // Move Straight, Turn Right, Continue Move Straight
            gpio_set_irq_enabled_with_callback(BINFRARED_D0_PIN, GPIO_IRQ_EDGE_RISE, true, &interrupt_callback);
            xSemaphoreGive(straight_path_task_semaphore);
            if (xSemaphoreTake(wall_semaphore, pdMS_TO_TICKS(5000)) == pdPASS)
            {
                direction = EAST;
                xMessageBufferSend(
                    g_leftWheelBuffer,
                    (void *)&direction,
                    sizeof(direction),
                    0);
                xMessageBufferSend(
                    g_rightWheelBuffer,
                    (void *)&direction,
                    sizeof(direction),
                    0);
                g_wall_detected = false;
            }
            xSemaphoreTake(main_task_semaphore, portMAX_DELAY);
            xSemaphoreGive(straight_path_task_semaphore);
            xSemaphoreTake(main_task_semaphore, portMAX_DELAY);
            gpio_set_irq_enabled_with_callback(BINFRARED_D0_PIN, GPIO_IRQ_EDGE_RISE, false, &interrupt_callback);
            break;
        case '5': // Move Straight, Turn Left, Continue Move Straight
            gpio_set_irq_enabled_with_callback(BINFRARED_D0_PIN, GPIO_IRQ_EDGE_RISE, true, &interrupt_callback);
            xSemaphoreGive(straight_path_task_semaphore);
            if (xSemaphoreTake(wall_semaphore, pdMS_TO_TICKS(5000)) == pdPASS)
            {
                direction = WEST;
                xMessageBufferSend(
                    g_leftWheelBuffer,
                    (void *)&direction,
                    sizeof(direction),
                    0);
                xMessageBufferSend(
                    g_rightWheelBuffer,
                    (void *)&direction,
                    sizeof(direction),
                    0);
                g_wall_detected = false;
            }
            xSemaphoreTake(main_task_semaphore, portMAX_DELAY);
            xSemaphoreGive(straight_path_task_semaphore);
            xSemaphoreTake(main_task_semaphore, portMAX_DELAY);
            gpio_set_irq_enabled_with_callback(BINFRARED_D0_PIN, GPIO_IRQ_EDGE_RISE, false, &interrupt_callback);
            break;
        case '6': // Barcode
            gpio_set_irq_enabled_with_callback(LINFRARED_PIN, GPIO_IRQ_EDGE_RISE, false, &interrupt_callback);
            gpio_set_irq_enabled_with_callback(RINFRARED_PIN, GPIO_IRQ_EDGE_RISE, false, &interrupt_callback);
            gpio_set_irq_enabled_with_callback(LENCODER_PIN, GPIO_IRQ_EDGE_RISE, false, &interrupt_callback);
            gpio_set_irq_enabled_with_callback(RENCODER_PIN, GPIO_IRQ_EDGE_RISE, false, &interrupt_callback);
            xSemaphoreGive(straight_path_task_semaphore);
            xSemaphoreGive(barcode_task_sempahore);
            xSemaphoreTake(barcode_task_complete, portMAX_DELAY);
            xSemaphoreTake(main_task_semaphore, portMAX_DELAY);
            break;
        case '7': // Ultrasonic
            gpio_set_irq_enabled_with_callback(ECHO_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &interrupt_callback);
            xSemaphoreGive(straight_path_task_semaphore);
            if (xSemaphoreTake(object_semaphore, pdMS_TO_TICKS(5000)) == pdPASS)
            {
                gpio_set_irq_enabled_with_callback(ECHO_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, false, &interrupt_callback);
                direction = SOUTH;
                xMessageBufferSend(
                    g_leftWheelBuffer,
                    (void *)&direction,
                    sizeof(direction),
                    0);
                xMessageBufferSend(
                    g_rightWheelBuffer,
                    (void *)&direction,
                    sizeof(direction),
                    0);
                g_object_detected = false;
            }
            xSemaphoreTake(main_task_semaphore, portMAX_DELAY);
            xSemaphoreGive(straight_path_task_semaphore);
            xSemaphoreTake(main_task_semaphore, portMAX_DELAY);
            gpio_set_irq_enabled_with_callback(ECHO_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, false, &interrupt_callback);
        default:
            break;
        }
        gpio_set_irq_enabled_with_callback(LINFRARED_PIN, GPIO_IRQ_EDGE_RISE, false, &interrupt_callback);
        gpio_set_irq_enabled_with_callback(RINFRARED_PIN, GPIO_IRQ_EDGE_RISE, false, &interrupt_callback);
        gpio_set_irq_enabled_with_callback(LENCODER_PIN, GPIO_IRQ_EDGE_RISE, false, &interrupt_callback);
        gpio_set_irq_enabled_with_callback(RENCODER_PIN, GPIO_IRQ_EDGE_RISE, false, &interrupt_callback);
        gpio_set_irq_enabled_with_callback(BINFRARED_D0_PIN, GPIO_IRQ_EDGE_RISE, false, &interrupt_callback);
    }
}

void start_tasks()
{
    left_wheel_task_complete = xSemaphoreCreateBinary();
    right_wheel_task_complete = xSemaphoreCreateBinary();
    main_task_semaphore = xSemaphoreCreateBinary();
    TaskHandle_t task_main;
    xTaskCreate(main_task, "main thread", configMINIMAL_STACK_SIZE, NULL, MAIN_TASK, &task_main);

    // Start the webserver task
    TaskHandle_t task_webserver;
    xTaskCreate(webserver_task, "web server thread", configMINIMAL_STACK_SIZE, NULL, WEB_TASK_PRIORITY, &task_webserver);

    send_choose_duty_cycle_buffer = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);
    receive_choose_duty_cycle_buffer = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);
    TaskHandle_t task_choose_duty_cycle;
    xTaskCreate(choose_duty_cycle_task, "choose duty cycle thread", configMINIMAL_STACK_SIZE, NULL, MAIN_TASK, &task_choose_duty_cycle);

    send_sync_duty_cycle_buffer = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);
    receive_sync_duty_cycle_buffer = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);
    TaskHandle_t task_sync_duty_cycle;
    xTaskCreate(sync_wheel_task, "sync duty cycle thread", configMINIMAL_STACK_SIZE, NULL, MAIN_TASK, &task_sync_duty_cycle);

    wall_semaphore = xSemaphoreCreateBinary();
    straight_path_task_semaphore = xSemaphoreCreateBinary();
    TaskHandle_t task_straight_path;
    xTaskCreate(straight_path_task, "straight path thread", configMINIMAL_STACK_SIZE, NULL, MAIN_TASK, &task_straight_path);

    magnetometer_task = xSemaphoreCreateBinary();
    TaskHandle_t task_magnetometer;
    xTaskCreate(snap_threshold_task, "snap threshold thread", configMINIMAL_STACK_SIZE, NULL, MAIN_TASK, &task_magnetometer);

    g_leftWheelBuffer = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);
    TaskHandle_t task_left_wheel;
    xTaskCreate(left_wheel_task, "left wheel thread", configMINIMAL_STACK_SIZE, NULL, MAIN_TASK, &task_left_wheel);

    g_rightWheelBuffer = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);
    TaskHandle_t task_right_wheel;
    xTaskCreate(right_wheel_task, "right wheel thread", configMINIMAL_STACK_SIZE, NULL, MAIN_TASK, &task_right_wheel);

    object_semaphore = xSemaphoreCreateBinary();
    TaskHandle_t ultrasonic_task;
    xTaskCreate(generate_sound_task, "ultrasonic thread", configMINIMAL_STACK_SIZE, NULL, MAIN_TASK, &ultrasonic_task);

    barcode_task_sempahore = xSemaphoreCreateBinary();
    barcode_task_complete = xSemaphoreCreateBinary();
    TaskHandle_t task_barcode;
    xTaskCreate(barcode_task, "barcode thread", configMINIMAL_STACK_SIZE, NULL, MAIN_TASK, &task_barcode);

    vTaskStartScheduler();
}
// End of init_task.c

int main()
{
    stdio_init_all();
    high_pin_init();

    infrared_init();
    encoder_init();
    magnetometer_init();
    ultrasonic_init();
    motor_control_init();

    start_tasks();
    return 0;
}