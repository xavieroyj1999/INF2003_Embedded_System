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

#include <FreeRTOSConfig.h>
#include <declarations.h>
#include <global_variables.h>
#include <function_prototype.h>
#include <barcode.h>
#include <initialize.h>
#include <magnometer.h>

void interrupt_callback(uint gpio, uint32_t events) {
    // Right Encoder Distance Counter
    uint32_t current_time = time_us_32();
    if (events & GPIO_IRQ_EDGE_RISE && gpio == RENCODER_PIN) {
        static uint32_t rencoder_last_time = 0;
        if (current_time - rencoder_last_time > DEBOUNCE_TIME) {
            g_distance_travelled += ONE_INTERRUPT;
            rencoder_last_time = current_time;
            g_right_encoder_interrupts++;
        }
    }

    // Left Encoder Distance Counter
    if (events & GPIO_IRQ_EDGE_RISE && gpio == LENCODER_PIN) {
        static uint32_t lencoder_last_time = 0;
        if (current_time - lencoder_last_time > DEBOUNCE_TIME) {
            g_distance_travelled += ONE_INTERRUPT;
            lencoder_last_time = current_time;
            g_left_encoder_interrupts++;
        }
    }

    // Left Infrared Sensor to readjust degree upon hitting a wall
    if (events & GPIO_IRQ_EDGE_RISE && gpio == LINFRARED_PIN) {
        static uint32_t linfrared_last_time = 0;
        if (current_time - linfrared_last_time > DEBOUNCE_TIME) {
            linfrared_last_time = current_time;
        }
        while(gpio_get(LINFRARED_PIN) == HIGH) {
            right_wheel_backward();
            gpio_set_irq_enabled_with_callback(RENCODER_PIN, GPIO_IRQ_EDGE_RISE, false, &interrupt_callback);
        }
        g_inital_degree = magneto_read();
        generateDegreeThresholds();
        right_wheel_stop();
        gpio_set_irq_enabled_with_callback(RENCODER_PIN, GPIO_IRQ_EDGE_RISE, true, &interrupt_callback);
    }

    // Right Infrared Sensor to readjust degree upon hitting a wall
    if (events & GPIO_IRQ_EDGE_RISE && gpio == RINFRARED_PIN) {
        static uint32_t rinfrared_last_time = 0;
        if (current_time - rinfrared_last_time > DEBOUNCE_TIME) {
            rinfrared_last_time = current_time;
        }
        while(gpio_get(RINFRARED_PIN) == HIGH) {
            left_wheel_backward();
            gpio_set_irq_enabled_with_callback(LENCODER_PIN, GPIO_IRQ_EDGE_RISE, true, &interrupt_callback);
        }
        g_inital_degree = magneto_read();
        generateDegreeThresholds();
        left_wheel_stop();
        gpio_set_irq_enabled_with_callback(LENCODER_PIN, GPIO_IRQ_EDGE_RISE, true, &interrupt_callback);
    }

    // Barcode Infrared Sensor to start reading barcode
    if (gpio == BINFRARED_PIN) {
        static bool last_edge_was_rising = false;
        static uint32_t last_edge_time = 0;
        static uint8_t odd_bit_index = 0;
        static uint8_t even_bit_index = 0;
        static uint32_t odd_time[5] = {0,0,0,0,0};
        static uint32_t even_time[4] = {0,0,0,0};

        if (current_time - last_edge_time < DEBOUNCE_TIME) {
            return;
        }

        // If rising edge, take time interval between rising and falling edge, then store the bit that will be << based on odd_bit_index
        if (events & GPIO_IRQ_EDGE_RISE && (odd_bit_index == 0)) {
            last_edge_time = time_us_32();
            last_edge_was_rising = true;
        }
        else if (events & GPIO_IRQ_EDGE_RISE && !last_edge_was_rising) {
            uint32_t time_interval = current_time - last_edge_time;
            even_time[even_bit_index] = time_interval;
            even_bit_index++;
            last_edge_was_rising = true;
        }
        else if (events & GPIO_IRQ_EDGE_FALL && last_edge_was_rising) {
            uint32_t time_interval = current_time - last_edge_time;
            odd_time[odd_bit_index] = time_interval;
            odd_bit_index++;
            last_edge_was_rising = false;
        }

        // If we have 9 bits, reset everything and decode the value
        if ((odd_bit_index + even_bit_index) >= 9) {
            uint8_t odd_bit = determine_odd_bit(odd_time);
            uint8_t even_bit = determine_even_bit(even_time);
            char decoded_value = code_39_decoder(odd_bit, even_bit);
            odd_bit_index = even_bit_index = 0;
            last_edge_was_rising = false;
            printf("Decoded Value: %c\n", decoded_value);
        }
        last_edge_time = current_time;
    }

    if (gpio == ECHO_PIN) {
        static uint64_t start_time = 0;
        uint64_t time_elapsed = 0;
        double distance = 0;
        if (events == GPIO_IRQ_EDGE_FALL)
        {
            time_elapsed = time_us_64() - start_time;
            // distance = calculate_distance(time_elapsed);
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
}

// start of motor.c
void left_wheel_forward() {
    gpio_put(INB1_PIN, HIGH);
    gpio_put(INB2_PIN, LOW);
}

void right_wheel_forward() {
    gpio_put(INA1_PIN, HIGH);
    gpio_put(INA2_PIN, LOW);
}

void left_wheel_backward() {
    gpio_put(INB1_PIN, LOW);
    gpio_put(INB2_PIN, HIGH);
}

void right_wheel_backward() {
    gpio_put(INA1_PIN, LOW);
    gpio_put(INA2_PIN, HIGH);
}

void left_wheel_stop() {
    gpio_put(INB1_PIN, LOW);
    gpio_put(INB2_PIN, LOW);
}

void right_wheel_stop() {
    gpio_put(INA1_PIN, LOW);
    gpio_put(INA2_PIN, LOW);
}

void set_wheel_duty_cycle(double duty_cycle_ratio) {
    if (duty_cycle_ratio > 1) {
        pwm_set_chan_level(GP5_SLICE, PWM_CHAN_B, PWM_FREQ);
        pwm_set_chan_level(GP0_SLICE, PWM_CHAN_A, (int) (PWM_FREQ / duty_cycle_ratio));
    } else {
        pwm_set_chan_level(GP5_SLICE, PWM_CHAN_B, (int) (PWM_FREQ * duty_cycle_ratio));
        pwm_set_chan_level(GP0_SLICE, PWM_CHAN_A, PWM_FREQ);
    }
}

static MessageBufferHandle_t send_choose_duty_cycle_buffer;
static MessageBufferHandle_t receive_choose_duty_cycle_buffer;

static MessageBufferHandle_t send_sync_duty_cycle_buffer;
static MessageBufferHandle_t receive_sync_duty_cycle_buffer;

static MessageBufferHandle_t g_leftWheelBuffer;
static MessageBufferHandle_t g_rightWheelBuffer;

static SemaphoreHandle_t left_wheel_task_complete;
static SemaphoreHandle_t right_wheel_task_complete;
static SemaphoreHandle_t main_task_semaphore;
static SemaphoreHandle_t magnetometer_task;
static SemaphoreHandle_t straight_path_task_semaphore;

void sync_wheel_task(void *pvParameters) {
    double duty_cycle = 0;
    while(true) {
        xMessageBufferReceive( 
            send_sync_duty_cycle_buffer,
            (void *) &duty_cycle,
            sizeof( duty_cycle ),
            portMAX_DELAY );
        printf("Syncing wheel with duty cycle: %f\n", duty_cycle);
        g_current_left_wheel_count = g_left_encoder_interrupts;
        g_current_right_wheel_count = g_right_encoder_interrupts;

        pwm_set_chan_level(GP5_SLICE, PWM_CHAN_B, (int) (PWM_FREQ * duty_cycle));
        pwm_set_chan_level(GP0_SLICE, PWM_CHAN_A, (int) (PWM_FREQ * duty_cycle));

        left_wheel_forward();
        right_wheel_forward();
        vTaskDelay(pdMS_TO_TICKS(10000));
        left_wheel_stop();
        right_wheel_stop();

        uint16_t left_wheel_travelled = g_left_encoder_interrupts - g_current_left_wheel_count;
        uint16_t right_wheel_travelled = g_right_encoder_interrupts - g_current_right_wheel_count; 
        double duty_cycle_ratio = (double) left_wheel_travelled/ right_wheel_travelled;

        set_wheel_duty_cycle(duty_cycle_ratio);

        xMessageBufferSend(
            receive_sync_duty_cycle_buffer,
            (void *) &duty_cycle_ratio,
            sizeof( duty_cycle_ratio ),
            portMAX_DELAY );
    }
}

void choose_duty_cycle_task(void *pvParameters) {
    char duty_cycle_selection = '\0';
    double duty_cycle_ratio = 1;

    while (true) {
        xMessageBufferReceive( 
            send_choose_duty_cycle_buffer,
            (void *) &duty_cycle_ratio,
            sizeof( duty_cycle_ratio ),
            portMAX_DELAY );

        printf("Choose duty cycle (0: 0%%, 1: 25%%, 2: 50%%, 3: 75%%, 4: 100%%): ");
        while(true) {
            duty_cycle_selection = getchar();
            if ( duty_cycle_selection == '0' || duty_cycle_selection == '1' || duty_cycle_selection == '2' || duty_cycle_selection == '3' || duty_cycle_selection == '4') break;
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

        xMessageBufferSend(receive_choose_duty_cycle_buffer, &duty_cycle, sizeof(duty_cycle), portMAX_DELAY);
    }
}

void straight_path_task(void* pvParameters) {
    u_int16_t current_left_wheel_count = 0;
    u_int16_t current_right_wheel_count = 0;
    while(true) {
        xSemaphoreTake(straight_path_task_semaphore, portMAX_DELAY);
        current_left_wheel_count = g_left_encoder_interrupts;
        current_right_wheel_count = g_right_encoder_interrupts;
        left_wheel_forward();
        right_wheel_forward();
        vTaskDelay(pdMS_TO_TICKS(10000));
        left_wheel_stop();
        right_wheel_stop();
        printf("Left wheel travelled: %d\n", g_left_encoder_interrupts - current_left_wheel_count);
        printf("Right wheel travelled: %d\n", g_right_encoder_interrupts - current_right_wheel_count);
        xSemaphoreGive(main_task_semaphore);
    }
}

void left_wheel_task(void* pvParameters) {
    enum directionEvent direction = STAY;
    u_int16_t current_left_wheel_count = 0;
    while (true) {
        xMessageBufferReceive(
            g_leftWheelBuffer,
            (void *) &direction,
            sizeof( direction ),
            portMAX_DELAY );
        current_left_wheel_count = g_left_encoder_interrupts;
        if (direction == NORTH || direction == EAST) {
                while (true) {                
                    left_wheel_forward();
                    vTaskDelay(50);
                    if (g_left_encoder_interrupts - current_left_wheel_count > 6) break;
                }
            }
            else if (direction == WEST) {
                while (true) {
                    left_wheel_backward();
                    vTaskDelay(50);
                    if (g_left_encoder_interrupts - current_left_wheel_count > 6) break;
                }
            }
            else if (direction == SOUTH) {
                while (true) {
                    left_wheel_backward();
                    vTaskDelay(50);
                    if (g_left_encoder_interrupts - current_left_wheel_count > 12) break;
                }
            }
            else if (direction == STAY) {
                left_wheel_stop();
            }
        left_wheel_stop();
        vTaskDelay(100);
        xSemaphoreGive(left_wheel_task_complete);
    }
}

void right_wheel_task(void *pvParameters) {
    enum directionEvent direction = STAY;
    u_int16_t current_right_wheel_count = 0;
    while (true) {
        xMessageBufferReceive(
            g_rightWheelBuffer,
            (void *) &direction,
            sizeof( direction ),
            portMAX_DELAY );
        current_right_wheel_count = g_right_encoder_interrupts;
        if (direction == NORTH || direction == WEST) {
                while (true) {
                    right_wheel_forward();
                    vTaskDelay(50);
                    if (g_right_encoder_interrupts - current_right_wheel_count > 6) break;
                }
            }
            else if (direction == EAST) {
                while (true) {
                    right_wheel_backward();
                    vTaskDelay(50);
                    if (g_right_encoder_interrupts - current_right_wheel_count > 6) break;
                }
            }
            else if (direction == SOUTH) {
                while (true) {
                    right_wheel_forward();
                    vTaskDelay(50);
                    if (g_right_encoder_interrupts - current_right_wheel_count > 12) break;
                }
            }
            else if (direction == STAY) {
                right_wheel_stop();
            }
            right_wheel_stop();
            vTaskDelay(100);
            xSemaphoreGive(right_wheel_task_complete);
    }
}

void snap_threshold_task() {
    while (true) {
        xSemaphoreTake(magnetometer_task, portMAX_DELAY);
        float current_degree = magneto_read();
        float degree_diffrence = findNearestDegreeDifference(current_degree);
        if (degree_diffrence > 10) {
            while(degree_diffrence > 10)
            {
                current_degree = magneto_read();
                degree_diffrence = findNearestDegreeDifference(current_degree);
                vTaskDelay(10);
                left_wheel_backward();
            }
        } else if (degree_diffrence < -10) {
            while (degree_diffrence < -10) {
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
// end of motor.c

// Start of init_task.c
#define MAIN_TASK (tskIDLE_PRIORITY + 1UL)

void main_task() {
    double left_wheel_duty_cycle = 0;
    double right_wheel_duty_cycle = 0;
    double duty_cycle = 0;
    double duty_cycle_ratio = 1;
    char event_to_perform = '\0';
    enum directionEvent direction = STAY;
    g_initial_degree = magneto_read();
    sleep_ms(5000);
    while (1) {
        printf("Choose what do:\n1. choose duty cycle\n2. sync wheel\n3. Straight Path\n4. Left Turn\n5. Right Turn\n6. Barcode\n7. Ultrasonic");
        event_to_perform = getchar();

        switch (event_to_perform)
        {
        case '1':
            xMessageBufferSend(
                send_choose_duty_cycle_buffer,
                (void *) &duty_cycle_ratio,
                sizeof( duty_cycle_ratio ),
                portMAX_DELAY );
            xMessageBufferReceive( 
                receive_choose_duty_cycle_buffer,
                (void *) &duty_cycle,
                sizeof( duty_cycle ),
                portMAX_DELAY );            
            printf("Duty cycle: %f\n", duty_cycle);
            break;
        case '2':
            xMessageBufferSend(
                send_sync_duty_cycle_buffer,
                (void *) &duty_cycle_ratio,
                sizeof( duty_cycle_ratio ),
                portMAX_DELAY );
            xMessageBufferReceive( 
                receive_sync_duty_cycle_buffer,
                (void *) &duty_cycle_ratio,
                sizeof( duty_cycle_ratio ),
                portMAX_DELAY );
            printf("Duty cycle ratio: %f\n", duty_cycle_ratio);
            break;
        case '3':
            xSemaphoreGive(straight_path_task_semaphore);
            xSemaphoreTake(main_task_semaphore, portMAX_DELAY);
            break;
        case '4':
            direction = NORTH;
            xMessageBufferSend(
                g_leftWheelBuffer,
                (void *) &direction,
                sizeof( direction ),
                portMAX_DELAY );
            xMessageBufferSend(
                g_rightWheelBuffer,
                (void *) &direction,
                sizeof( direction ),
                portMAX_DELAY );
            xSemaphoreTake(left_wheel_task_complete, portMAX_DELAY);
            xSemaphoreTake(right_wheel_task_complete, portMAX_DELAY);

            xSemaphoreGive(magnetometer_task);
            xSemaphoreTake(main_task_semaphore, portMAX_DELAY);

            direction = EAST;
            xMessageBufferSend(
                g_leftWheelBuffer,
                (void *) &direction,
                sizeof( direction ),
                portMAX_DELAY );
            xMessageBufferSend(
                g_rightWheelBuffer,
                (void *) &direction,
                sizeof( direction ),
                portMAX_DELAY );
            xSemaphoreTake(left_wheel_task_complete, portMAX_DELAY);
            xSemaphoreTake(right_wheel_task_complete, portMAX_DELAY);
            
            xSemaphoreGive(magnetometer_task);
            xSemaphoreTake(main_task_semaphore, portMAX_DELAY);
        default:
            break;
        }
    }
}

void start_tasks() {
    left_wheel_task_complete = xSemaphoreCreateBinary();
    right_wheel_task_complete = xSemaphoreCreateBinary();
    main_task_semaphore = xSemaphoreCreateBinary();
    TaskHandle_t task_main;
    xTaskCreate(main_task, "main thread", configMINIMAL_STACK_SIZE, NULL, MAIN_TASK, &task_main);

    send_choose_duty_cycle_buffer = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);
    receive_choose_duty_cycle_buffer = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);
    TaskHandle_t task_choose_duty_cycle;
    xTaskCreate(choose_duty_cycle_task, "choose duty cycle thread", configMINIMAL_STACK_SIZE, NULL, MAIN_TASK, &task_choose_duty_cycle);

    send_sync_duty_cycle_buffer = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);
    receive_sync_duty_cycle_buffer = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);
    TaskHandle_t task_sync_duty_cycle;
    xTaskCreate(sync_wheel_task, "sync duty cycle thread", configMINIMAL_STACK_SIZE, NULL, MAIN_TASK, &task_sync_duty_cycle);

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

    vTaskStartScheduler();
}
// End of init_task.c

int main() {
    stdio_init_all();
    high_pin_init();

    encoder_init();
    magnetometer_init();
    motor_control_init();

    gpio_set_irq_enabled_with_callback(LENCODER_PIN, GPIO_IRQ_EDGE_RISE, true, &interrupt_callback);
    gpio_set_irq_enabled_with_callback(RENCODER_PIN, GPIO_IRQ_EDGE_RISE, true, &interrupt_callback);

    start_tasks();
    return 0;
}