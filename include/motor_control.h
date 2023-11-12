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
                    if (g_left_encoder_interrupts - current_left_wheel_count > 5) break;
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
        while(findNearestDegreeDifference(magneto_read()) > 5) left_wheel_backward();
        left_wheel_stop();
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
            while (findNearestDegreeDifference(magneto_read()) < -10) right_wheel_backward();
            right_wheel_stop();
    }
}