#define USER_INPUT_TASK (tskIDLE_PRIORITY + 1UL)
#define MAGNETOMETER_TASK (tskIDLE_PRIORITY + 1UL)
#define PRINT_TASK (tskIDLE_PRIORITY + 10UL)

void print_task(void* pvParameters) {
    while(1) {
        vTaskDelay(1000);
        printf("Distance: %d cm\n", g_distance_travelled);
        printf("Left Speed: %d interrupts/second\n", g_left_encoder_interrupts);
        printf("Right Speed: %d interrupts/second\n", g_right_encoder_interrupts);
        printf("Degree: %f\n", g_degree);
        g_left_encoder_interrupts = 0;
        g_right_encoder_interrupts = 0;
    }
}

void start_tasks() {
    TaskHandle_t task_user_selection;
    xTaskCreate(user_selection_task, "user selection task", configMINIMAL_STACK_SIZE, NULL, USER_INPUT_TASK, &task_user_selection);

    TaskHandle_t task_magneto_read;
    xTaskCreate(magneto_read_task, "magneto thread", configMINIMAL_STACK_SIZE, NULL, MAGNETOMETER_TASK, &task_magneto_read);

    // TaskHandle_t task_print;
    // xTaskCreate(print_task, "print thread", configMINIMAL_STACK_SIZE, NULL, PRINT_TASK, &task_print);

    vTaskStartScheduler();
}

void start_interrupts() {
    gpio_set_irq_enabled_with_callback(LENCODER_PIN, GPIO_IRQ_EDGE_RISE, true, &interrupt_callback);
    gpio_set_irq_enabled_with_callback(RENCODER_PIN, GPIO_IRQ_EDGE_RISE, true, &interrupt_callback);
    gpio_set_irq_enabled_with_callback(LINFRARED_PIN, GPIO_IRQ_EDGE_RISE, true, &interrupt_callback);
    gpio_set_irq_enabled_with_callback(RINFRARED_PIN, GPIO_IRQ_EDGE_RISE, true, &interrupt_callback);
    gpio_set_irq_enabled_with_callback(BINFRARED_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &interrupt_callback);
}