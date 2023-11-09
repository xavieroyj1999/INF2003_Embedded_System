void control_wheels(enum directionEvent direction){
    g_initial_degree = g_degree;
    pwm_set_chan_level(GP5_SLICE, PWM_CHAN_B, PWM_FREQ * g_duty_cycle);
    pwm_set_chan_level(GP0_SLICE, PWM_CHAN_A, PWM_FREQ * g_duty_cycle);
    if (direction == NORTH)
    {
        gpio_put(INA1_PIN, HIGH);
        gpio_put(INA2_PIN, LOW);

        gpio_put(INB1_PIN, HIGH);
        gpio_put(INB2_PIN, LOW);
    }
    else if (direction == EAST)
    {
        gpio_put(INA1_PIN, LOW);
        gpio_put(INA2_PIN, HIGH);

        gpio_put(INB1_PIN, HIGH);
        gpio_put(INB2_PIN, LOW);
    }
    else if (direction == SOUTH)
    {
        gpio_put(INA1_PIN, LOW);
        gpio_put(INA2_PIN, HIGH);

        gpio_put(INB1_PIN, LOW);
        gpio_put(INB2_PIN, HIGH);
    }
    else if (direction == WEST)
    {
        gpio_put(INA1_PIN, HIGH);
        gpio_put(INA2_PIN, LOW);

        gpio_put(INB1_PIN, LOW);
        gpio_put(INB2_PIN, HIGH);
    }
    else if (direction == STAY)
    {
        gpio_put(INA1_PIN, LOW);
        gpio_put(INA2_PIN, LOW);

        gpio_put(INB1_PIN, LOW);
        gpio_put(INB2_PIN, LOW);
    }
}

void user_selection_task(void* pvParameters) {
    char duty_cycle_selection;
    char direction_selection;

    while (1) {
        printf("Choose duty cycle (0: 0%%, 1: 25%%, 2: 50%%, 3: 75%%, 4: 100%%): ");
        duty_cycle_selection = getchar();

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
            default:
                printf("Invalid duty cycle\n");
                continue;
        }

        printf("Choose direction (0: STAY, 1: NORTH, 2: EAST, 3: SOUTH, 4: WEST): ");
        direction_selection = getchar();

        switch (direction_selection)
        {
            case '0':
                control_wheels(STAY);
                break;
            case '1':
                control_wheels(NORTH);
                break;
            case '2':
                control_wheels(EAST);
                while (!right_angle_turn());
                control_wheels(STAY);
                g_initial_degree = g_degree;
                break;
            case '3':
                control_wheels(SOUTH);
                break;
            case '4':
                control_wheels(WEST);
                while (!right_angle_turn());
                control_wheels(STAY);
                g_initial_degree = g_degree;
                break;
            default:
                printf("Invalid direction\n");
        }
    }
}