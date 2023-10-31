#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "message_buffer.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"

// Motor Configs
#define PWM_FREQ 12500

// Motor A Pins
#define GP5_SLICE 2
#define ENA_PWM 5
#define INA1 3
#define INA2 4

// Motor B Pins
#define GP0_SLICE 0
#define ENB_PWM 0
#define INB1 1
#define INB2 2

// Active low and high
#define LOW 0
#define HIGH 1

// Define Task Priorities
#define USER_INPUT_TASK (tskIDLE_PRIORITY + 1UL)

enum directionEvent {
    STAY = 0x00,
    NORTH = 0x01,
    EAST = 0x02,
    SOUTH = 0x04,
    WEST = 0x08
};

void init_gpio_motor(){
    // Motor A
    gpio_set_function(ENA_PWM, GPIO_FUNC_PWM); // PWM to control speed.
    gpio_init(INA1);
    gpio_set_dir(INA1, GPIO_OUT); // Direction control - Set output to pin INA1 or 2 accordingly.
    gpio_put(INA1, LOW); // Ensure that the car is stationary.
    gpio_init(INA2);
    gpio_set_dir(INA2, GPIO_OUT);
    gpio_put(INA2, LOW);

    // Motor B
    gpio_set_function(ENB_PWM, GPIO_FUNC_PWM);
    gpio_init(INB1);
    gpio_set_dir(INB1, GPIO_OUT);
    gpio_put(INB1, LOW);
    gpio_init(INB2);
    gpio_set_dir(INB2, GPIO_OUT);
    gpio_put(INB2, LOW);
}

// Motor A
void init_motor_a(){
    pwm_set_clkdiv(GP5_SLICE, 100); // Sets clock divider for specified slice to control PWM frequency: Divide default clock value by 100.
    pwm_set_wrap(GP5_SLICE, PWM_FREQ); // Sets wrap value which determines the period of the PWM signal.
    pwm_set_chan_level(GP5_SLICE, PWM_CHAN_B, 0); // Set the level of a particular channel within the specified PWM slice (0 = Duty Cycle 0%)
    pwm_set_enabled(GP5_SLICE, true);
}

// Motor B
void init_motor_b(){
    pwm_set_clkdiv(GP0_SLICE, 100);
    pwm_set_wrap(GP0_SLICE, PWM_FREQ);
    pwm_set_chan_level(GP0_SLICE, PWM_CHAN_A, 0);
    pwm_set_enabled(GP0_SLICE, true);
}

// Wheel Control
void control_wheels(double duty_cycle_a, double duty_cycle_b, enum directionEvent direction){
    pwm_set_chan_level(GP5_SLICE, PWM_CHAN_B, PWM_FREQ * duty_cycle_a);
    pwm_set_chan_level(GP0_SLICE, PWM_CHAN_A, PWM_FREQ * duty_cycle_b);
    if (direction == NORTH)
    {
        gpio_put(INA1, HIGH);
        gpio_put(INA2, LOW);

        gpio_put(INB1, HIGH);
        gpio_put(INB2, LOW);
    }
    else if (direction == EAST)
    {
        gpio_put(INA1, LOW);
        gpio_put(INA2, HIGH);

        gpio_put(INB1, HIGH);
        gpio_put(INB2, LOW);
    }
    else if (direction == SOUTH)
    {
        gpio_put(INA1, LOW);
        gpio_put(INA2, HIGH);

        gpio_put(INB1, LOW);
        gpio_put(INB2, HIGH);
    }
    else if (direction == WEST)
    {
        gpio_put(INA1, HIGH);
        gpio_put(INA2, LOW);

        gpio_put(INB1, LOW);
        gpio_put(INB2, HIGH);
    }
    else if (direction == STAY)
    {
        gpio_put(INA1, LOW);
        gpio_put(INA2, LOW);

        gpio_put(INB1, LOW);
        gpio_put(INB2, LOW);
    }
}

// User Selection
void user_selection_task(void* pvParameters) {
    char duty_cycle_selection = '0';
    char direction_selection = '0';

    while (1) {
        printf("Choose duty cycle (0: 0%%, 1: 25%%, 2: 50%%, 3: 75%%, 4: 100%%): ");
        duty_cycle_selection = getchar();

        double duty_cycle;
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
            default:
                printf("Invalid duty cycle\n");
                continue;
        }

        printf("Choose direction (0: STAY, 1: NORTH, 2: EAST, 3: SOUTH, 4: WEST): ");
        direction_selection = getchar();

        switch (direction_selection)
        {
            case '0':
                control_wheels(duty_cycle, duty_cycle, STAY);
                break;
            case '1':
                control_wheels(duty_cycle, duty_cycle, NORTH);
                break;
            case '2':
                control_wheels(duty_cycle, duty_cycle, EAST);
                break;
            case '3':
                control_wheels(duty_cycle, duty_cycle, SOUTH);
                break;
            case '4':
                control_wheels(duty_cycle, duty_cycle, WEST);
                break;
            default:
                printf("Invalid direction\n");
        }
    }
}

// Start Tasks
void start_tasks() {

    // Create user input tasks
    TaskHandle_t user_input_task;

    // Start FreeRTOS scheduler
    xTaskCreate(user_selection_task, "user selection task", configMINIMAL_STACK_SIZE, NULL, USER_INPUT_TASK, &user_input_task);

    vTaskStartScheduler();
}

int main() {
    stdio_init_all();
    init_gpio_motor();
    init_motor_a();
    init_motor_b();

    start_tasks();

    return 0;
}
