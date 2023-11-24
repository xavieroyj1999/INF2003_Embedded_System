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

void choose_duty_cycle() {
    char duty_cycle_selection = '\0';

    printf("Choose duty cycle (0: 0%%, 1: 25%%, 2: 50%%, 3: 75%%, 4: 100%%): ");
    while(true) {
        duty_cycle_selection = getchar();
        if ( duty_cycle_selection == '0' || duty_cycle_selection == '1' || duty_cycle_selection == '2' || duty_cycle_selection == '3' || duty_cycle_selection == '4') break;
        printf("Invalid duty cycle\n");
    }

    switch (duty_cycle_selection)
    {
        case '0':
            g_duty_cycle = 0;
            break;
        case '1':
            g_duty_cycle = 0.25;
            break;
        case '2':
            g_duty_cycle = 0.5;
            break;
        case '3':
            g_duty_cycle = 0.75;
            break;
        case '4':
            g_duty_cycle = 1;
            break;
    }

    pwm_set_chan_level(GP5_SLICE, PWM_CHAN_B, PWM_FREQ * (g_duty_cycle - 0.05));
    pwm_set_chan_level(GP0_SLICE, PWM_CHAN_A, PWM_FREQ * g_duty_cycle);
}

char choose_direction() {
    char event_to_perform = '\0';
    while(true) {
        printf("Choose direction (0: STAY, 1: NORTH, 2: EAST, 3: SOUTH, 4: WEST): ");
        event_to_perform = getchar();
        if ( event_to_perform == '0' || event_to_perform == '1' || event_to_perform == '2' || event_to_perform == '3' || event_to_perform == '4') break;
        printf("Invalid direction\n");
    }
    return event_to_perform;
}

void left_wheel_task(void* pvParameters) {
    enum directionEvent direction = STAY;
    uint8_t current_left_wheel_count = 0;
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
        while(findNearestDegreeDifference(magneto_read()) > 5)
        {
            printf("mag: %f\n", magneto_read());
            vTaskDelay(100);
            left_wheel_backward();
        }
        left_wheel_stop();
        vTaskSuspend(NULL);
    }
}

void right_wheel_task(void *pvParameters) {
    enum directionEvent direction = STAY;
    uint8_t current_right_wheel_count = 0;
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
                    if (g_left_encoder_interrupts - current_right_wheel_count > 6) break;
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
            while (findNearestDegreeDifference(magneto_read()) < -10) {
                printf("mag: %f\n", magneto_read());
                vTaskDelay(100);
                right_wheel_backward();
            }
            right_wheel_stop();
            vTaskSuspend(NULL);       
    }
}