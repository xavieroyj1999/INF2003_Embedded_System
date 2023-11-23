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
    /* We can only reduce the duty cycle instead of increasing as, if the duty cycle is set to 100%,
    the motor not be able to further increase it's speed
    
    Formula used to sync wheel was ratio, left/right well
    if right wheel is faster than left wheel, reduce right wheel speed
    else if left wheel is faster than right wheel, reduce left wheel speed*/
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
            g_send_sync_duty_cycle_buffer,
            (void *)&duty_cycle,
            sizeof(duty_cycle),
            portMAX_DELAY);

        g_current_left_wheel_count = g_left_encoder_interrupts;
        g_current_right_wheel_count = g_right_encoder_interrupts;

        pwm_set_chan_level(GP5_SLICE, PWM_CHAN_B, (int)(PWM_FREQ * duty_cycle));
        pwm_set_chan_level(GP0_SLICE, PWM_CHAN_A, (int)(PWM_FREQ * duty_cycle));

        // Move both wheels forward for 10 seconds
        left_wheel_forward();
        right_wheel_forward();
        vTaskDelay(pdMS_TO_TICKS(10000));
        left_wheel_stop();
        right_wheel_stop();

        // Determine the ratio based on the encoder interrupts
        uint16_t left_wheel_travelled = g_left_encoder_interrupts - g_current_left_wheel_count;
        uint16_t right_wheel_travelled = g_right_encoder_interrupts - g_current_right_wheel_count;
        double duty_cycle_ratio = (double)left_wheel_travelled / right_wheel_travelled;

        set_wheel_duty_cycle(duty_cycle_ratio);

        xMessageBufferSend(
            g_receive_sync_duty_cycle_buffer,
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
            g_send_choose_duty_cycle_buffer,
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

        xMessageBufferSend(g_receive_choose_duty_cycle_buffer,
            &duty_cycle,
            sizeof(duty_cycle),
            0);
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
                if (g_left_encoder_interrupts - current_left_wheel_count > 4) break;
            }
        }
        else if (direction == WEST)
        {
            while (true)
            {
                left_wheel_backward();
                vTaskDelay(50);
                if (g_left_encoder_interrupts - current_left_wheel_count > 4) break;
            }
        }
        else if (direction == SOUTH)
        {
            while (true)
            {
                left_wheel_backward();
                vTaskDelay(50);
                if (g_left_encoder_interrupts - current_left_wheel_count > 9) break;
            }
        }
        else if (direction == STAY)
        {
            left_wheel_stop();
        }
        left_wheel_stop();
        xSemaphoreGive(g_left_wheel_task_complete);
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
                if (g_right_encoder_interrupts - current_right_wheel_count > 4) break;
            }
        }
        else if (direction == EAST)
        {
            while (true)
            {
                right_wheel_backward();
                vTaskDelay(50);
                if (g_right_encoder_interrupts - current_right_wheel_count > 4) break;
            }
        }
        else if (direction == SOUTH)
        {
            while (true)
            {
                right_wheel_forward();
                vTaskDelay(50);
                if (g_right_encoder_interrupts - current_right_wheel_count > 9) break;
            }
        }
        else if (direction == STAY)
        {
            right_wheel_stop();
        }
        right_wheel_stop();
        xSemaphoreGive(g_right_wheel_task_complete);
    }
}

void send_buffer_to_wheels(enum directionEvent direction)
{
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
}

void straight_path_task(void *pvParameters)
{
    u_int16_t current_left_wheel_count = 0;
    u_int16_t current_right_wheel_count = 0;
    u_int16_t count = 0;
    while (true)
    {
        xSemaphoreTake(g_straight_path_task_semaphore, portMAX_DELAY);
        current_left_wheel_count = g_left_encoder_interrupts;
        current_right_wheel_count = g_right_encoder_interrupts;
        left_wheel_forward();
        right_wheel_forward();

        // Travel straight for 5 seconds, but will be stopped if there is a wall or object detected
        while (count <= 100)
        {
            if (gb_object_detected || gb_wall_detected)
            {
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
        xSemaphoreGive(g_main_task_semaphore);
    }
}