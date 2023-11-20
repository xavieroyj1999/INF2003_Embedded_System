#define MAGNETOMETER_TASK (tskIDLE_PRIORITY + 1UL)
#define MAIN_TASK (tskIDLE_PRIORITY + 1UL)
#define CONTROL_WHEEL_TASK (tskIDLE_PRIORITY + 2UL)

#define mbaTASK_MESSAGE_BUFFER_SIZE ( 60 )

static MessageBufferHandle_t g_leftWheelBuffer;
static MessageBufferHandle_t g_rightWheelBuffer;

TaskHandle_t task_left_wheel;
TaskHandle_t task_right_wheel;
// TaskHandle_t task_ultrasonic;

void start_tasks() {
    TaskHandle_t task_main;
    xTaskCreate(main_task, "main thread", configMINIMAL_STACK_SIZE, NULL, MAIN_TASK, &task_main);

    g_leftWheelBuffer = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);
    xTaskCreate(left_wheel_task, "left wheel thread", configMINIMAL_STACK_SIZE, NULL, CONTROL_WHEEL_TASK, &task_left_wheel);

    g_rightWheelBuffer = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);
    xTaskCreate(right_wheel_task, "right wheel thread", configMINIMAL_STACK_SIZE, NULL, CONTROL_WHEEL_TASK, &task_right_wheel);

    // xTaskCreate(task_ultrasonic, "ultrasonic thread", configMINIMAL_STACK_SIZE, NULL, CONTROL_WHEEL_TASK, &task_ultrasonic);

    vTaskStartScheduler();
}

void main_task(void* pvParameters) {
    choose_duty_cycle();
    g_inital_degree = magneto_read();
    char event_to_perform = '\0';
    enum directionEvent direction_selection = STAY;

    while (true)
    {
        direction_selection = NORTH;
        xMessageBufferSend(
            g_leftWheelBuffer,
            (void *) &direction_selection,
            sizeof( direction_selection ),
            0 );
        xMessageBufferSend(
            g_rightWheelBuffer,
            (void *) &direction_selection,
            sizeof( direction_selection ),
            0 );
        vTaskSuspend(NULL);
    }
}

void start_interrupts() {
    gpio_set_irq_enabled_with_callback(LENCODER_PIN, GPIO_IRQ_EDGE_RISE, true, &interrupt_callback);
    gpio_set_irq_enabled_with_callback(RENCODER_PIN, GPIO_IRQ_EDGE_RISE, true, &interrupt_callback);
    gpio_set_irq_enabled_with_callback(LINFRARED_PIN, GPIO_IRQ_EDGE_RISE, true, &interrupt_callback);
    gpio_set_irq_enabled_with_callback(RINFRARED_PIN, GPIO_IRQ_EDGE_RISE, false, &interrupt_callback);  // only change to true if we need to detect right wall
    gpio_set_irq_enabled_with_callback(BINFRARED_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &interrupt_callback);
    gpio_set_irq_enabled_with_callback(ECHO_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, false, &interrupt_callback); // only change to true if we know what to do when we meet obstacle
}