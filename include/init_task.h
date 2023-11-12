#define MAGNETOMETER_TASK (tskIDLE_PRIORITY + 1UL)
#define MAIN_TASK (tskIDLE_PRIORITY + 1UL)
#define CONTROL_WHEEL_TASK (tskIDLE_PRIORITY + 2UL)

#define mbaTASK_MESSAGE_BUFFER_SIZE ( 60 )

static MessageBufferHandle_t g_eventBuffer;
static MessageBufferHandle_t g_leftWheelBuffer;
static MessageBufferHandle_t g_rightWheelBuffer;

void start_tasks() {
    g_eventBuffer = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);
    TaskHandle_t task_main;
    xTaskCreate(main_task, "main thread", configMINIMAL_STACK_SIZE, NULL, MAIN_TASK, &task_main);

    g_leftWheelBuffer = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);
    TaskHandle_t task_left_wheel;
    xTaskCreate(left_wheel_task, "left wheel thread", configMINIMAL_STACK_SIZE, NULL, CONTROL_WHEEL_TASK, &task_left_wheel);

    g_rightWheelBuffer = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);
    TaskHandle_t task_right_wheel;
    xTaskCreate(right_wheel_task, "right wheel thread", configMINIMAL_STACK_SIZE, NULL, CONTROL_WHEEL_TASK, &task_right_wheel);

    vTaskStartScheduler();
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

    pwm_set_chan_level(GP5_SLICE, PWM_CHAN_B, PWM_FREQ * g_duty_cycle);
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

void main_task(void* pvParameters) {
    choose_duty_cycle();
    g_inital_degree = magneto_read();
    char event_to_perform = '\0';
    enum directionEvent direction_selection = STAY;
    while (true)
    {
        event_to_perform = choose_direction();
        switch (event_to_perform) // 0: STAY, 1: NORTH, 2: EAST, 3: SOUTH, 4: WEST
        {
            case '0':
                direction_selection = STAY;
                break;
            case '1':
                direction_selection = NORTH;
                break;
            case '2':
                direction_selection = EAST;
                break;
            case '3':
                direction_selection = SOUTH;
                break;
            case '4':
                direction_selection = WEST;
                break;
            default:
                break;
        }
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
    }
}

void start_interrupts() {
    gpio_set_irq_enabled_with_callback(LENCODER_PIN, GPIO_IRQ_EDGE_RISE, true, &interrupt_callback);
    gpio_set_irq_enabled_with_callback(RENCODER_PIN, GPIO_IRQ_EDGE_RISE, true, &interrupt_callback);
    gpio_set_irq_enabled_with_callback(LINFRARED_PIN, GPIO_IRQ_EDGE_RISE, true, &interrupt_callback);
    gpio_set_irq_enabled_with_callback(RINFRARED_PIN, GPIO_IRQ_EDGE_RISE, true, &interrupt_callback);
    gpio_set_irq_enabled_with_callback(BINFRARED_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &interrupt_callback);
}