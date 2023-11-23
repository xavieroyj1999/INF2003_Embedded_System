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

    // Left Infrared Sensor detect wall
    if (events & GPIO_IRQ_EDGE_RISE && gpio == LINFRARED_PIN)
    {
        static uint32_t linfrared_last_time = 0;
        if (current_time - linfrared_last_time > DEBOUNCE_TIME)
        {
            linfrared_last_time = current_time;
            gpio_set_irq_enabled_with_callback(RENCODER_PIN, GPIO_IRQ_EDGE_RISE, false, &interrupt_callback);

            // As long as the left infrared sensor detects a wall, the car will turn right
            left_wheel_stop();
            right_wheel_backward();
            while (gpio_get(LINFRARED_PIN) == HIGH);
            left_wheel_forward();
            right_wheel_forward();

            gpio_set_irq_enabled_with_callback(RENCODER_PIN, GPIO_IRQ_EDGE_RISE, true, &interrupt_callback);
        }
    }

    // Right Infrared Sensor detect Wall
    if (events & GPIO_IRQ_EDGE_RISE && gpio == RINFRARED_PIN)
    {
        static uint32_t rinfrared_last_time = 0;
        if (current_time - rinfrared_last_time > DEBOUNCE_TIME)
        {
            rinfrared_last_time = current_time;
            gpio_set_irq_enabled_with_callback(LENCODER_PIN, GPIO_IRQ_EDGE_RISE, true, &interrupt_callback);

            // As long as the right infrared sensor detects a wall, the car will turn left
            right_wheel_stop();
            left_wheel_backward();
            while (gpio_get(RINFRARED_PIN) == HIGH);
            left_wheel_forward();
            right_wheel_forward();

            gpio_set_irq_enabled_with_callback(LENCODER_PIN, GPIO_IRQ_EDGE_RISE, true, &interrupt_callback);
        }
    }

    // Barcode Infrared Sensor Detect Wall
    if (events & GPIO_IRQ_EDGE_RISE && gpio == BINFRARED_D0_PIN)
    {
        static uint32_t bdinfrared_last_time = 0;
        if (current_time - bdinfrared_last_time > DEBOUNCE_TIME)
        {
            bdinfrared_last_time = current_time;
            gb_wall_detected = true;

            // As long as the barcode infrared sensor detects a wall, the car will reverse
            gpio_set_irq_enabled_with_callback(BINFRARED_D0_PIN, GPIO_IRQ_EDGE_RISE, false, &interrupt_callback);
            left_wheel_backward();
            right_wheel_backward();
            while (gpio_get(BINFRARED_D0_PIN) == HIGH);
            left_wheel_stop();
            right_wheel_stop();
            
            xSemaphoreGive(g_wall_semaphore);
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
            if (distance < 10)
            {
                xSemaphoreGive(g_object_semaphore);
                gb_object_detected = true;
            }
        }
        else if (events == GPIO_IRQ_EDGE_RISE)
        {
            start_time = time_us_64();
        }
    }
}

void disable_all_interrupts()
{
    gpio_set_irq_enabled_with_callback(LINFRARED_PIN, GPIO_IRQ_EDGE_RISE, false, &interrupt_callback);
    gpio_set_irq_enabled_with_callback(RINFRARED_PIN, GPIO_IRQ_EDGE_RISE, false, &interrupt_callback);
    gpio_set_irq_enabled_with_callback(LENCODER_PIN, GPIO_IRQ_EDGE_RISE, false, &interrupt_callback);
    gpio_set_irq_enabled_with_callback(RENCODER_PIN, GPIO_IRQ_EDGE_RISE, false, &interrupt_callback);
    gpio_set_irq_enabled_with_callback(BINFRARED_D0_PIN, GPIO_IRQ_EDGE_RISE, false, &interrupt_callback);
}

void enable_all_interrupts()
{
    gpio_set_irq_enabled_with_callback(LINFRARED_PIN, GPIO_IRQ_EDGE_RISE, true, &interrupt_callback);
    gpio_set_irq_enabled_with_callback(RINFRARED_PIN, GPIO_IRQ_EDGE_RISE, true, &interrupt_callback);
    gpio_set_irq_enabled_with_callback(LENCODER_PIN, GPIO_IRQ_EDGE_RISE, true, &interrupt_callback);
    gpio_set_irq_enabled_with_callback(RENCODER_PIN, GPIO_IRQ_EDGE_RISE, true, &interrupt_callback);
    gpio_set_irq_enabled_with_callback(BINFRARED_D0_PIN, GPIO_IRQ_EDGE_RISE, true, &interrupt_callback);
}

void enable_encoder_interrupts()
{
    gpio_set_irq_enabled_with_callback(LENCODER_PIN, GPIO_IRQ_EDGE_RISE, true, &interrupt_callback);
    gpio_set_irq_enabled_with_callback(RENCODER_PIN, GPIO_IRQ_EDGE_RISE, true, &interrupt_callback);
}

void enable_infrared_interrupts()
{
    gpio_set_irq_enabled_with_callback(LINFRARED_PIN, GPIO_IRQ_EDGE_RISE, true, &interrupt_callback);
    gpio_set_irq_enabled_with_callback(RINFRARED_PIN, GPIO_IRQ_EDGE_RISE, true, &interrupt_callback);
}

void enable_ultrasonic_interrupts()
{
    gpio_set_irq_enabled_with_callback(ECHO_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &interrupt_callback);
}

void start_tasks()
{
    g_left_wheel_task_complete = xSemaphoreCreateBinary();
    g_right_wheel_task_complete = xSemaphoreCreateBinary();
    g_main_task_semaphore = xSemaphoreCreateBinary();
    TaskHandle_t task_main;
    xTaskCreate(main_task, "main thread", configMINIMAL_STACK_SIZE, NULL, MAIN_TASK, &task_main);

    // Start the webserver task
    TaskHandle_t task_webserver;
    xTaskCreate(webserver_task, "web server thread", configMINIMAL_STACK_SIZE, NULL, WEB_TASK_PRIORITY, &task_webserver);

    g_send_choose_duty_cycle_buffer = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);
    g_receive_choose_duty_cycle_buffer = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);
    TaskHandle_t task_choose_duty_cycle;
    xTaskCreate(choose_duty_cycle_task, "choose duty cycle thread", configMINIMAL_STACK_SIZE, NULL, MAIN_TASK, &task_choose_duty_cycle);

    g_send_sync_duty_cycle_buffer = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);
    g_receive_sync_duty_cycle_buffer = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);
    TaskHandle_t task_sync_duty_cycle;
    xTaskCreate(sync_wheel_task, "sync duty cycle thread", configMINIMAL_STACK_SIZE, NULL, MAIN_TASK, &task_sync_duty_cycle);

    g_wall_semaphore = xSemaphoreCreateBinary();
    g_straight_path_task_semaphore = xSemaphoreCreateBinary();
    TaskHandle_t task_straight_path;
    xTaskCreate(straight_path_task, "straight path thread", configMINIMAL_STACK_SIZE, NULL, MAIN_TASK, &task_straight_path);

    g_leftWheelBuffer = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);
    TaskHandle_t task_left_wheel;
    xTaskCreate(left_wheel_task, "left wheel thread", configMINIMAL_STACK_SIZE, NULL, MAIN_TASK, &task_left_wheel);

    g_rightWheelBuffer = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);
    TaskHandle_t task_right_wheel;
    xTaskCreate(right_wheel_task, "right wheel thread", configMINIMAL_STACK_SIZE, NULL, MAIN_TASK, &task_right_wheel);

    g_object_semaphore = xSemaphoreCreateBinary();
    TaskHandle_t task_generate_ultrasonic;
    xTaskCreate(generate_ultrasonic_task, "ultrasonic thread", configMINIMAL_STACK_SIZE, NULL, MAIN_TASK, &task_generate_ultrasonic);

    g_barcode_task_sempahore = xSemaphoreCreateBinary();
    g_barcode_task_complete = xSemaphoreCreateBinary();
    TaskHandle_t task_barcode;
    xTaskCreate(barcode_task, "barcode thread", configMINIMAL_STACK_SIZE, NULL, MAIN_TASK, &task_barcode);

    vTaskStartScheduler();
}