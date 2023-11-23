void high_pin_init()
{
    gpio_init(ULTRASONIC_3V3_PIN);
    gpio_set_dir(ULTRASONIC_3V3_PIN, GPIO_OUT);
    gpio_put(ULTRASONIC_3V3_PIN, HIGH);

    gpio_init(LENCODER_3V3_PIN);
    gpio_set_dir(LENCODER_3V3_PIN, GPIO_OUT);
    gpio_put(LENCODER_3V3_PIN, HIGH);

    gpio_init(RENCODER_3V3_PIN);
    gpio_set_dir(RENCODER_3V3_PIN, GPIO_OUT);
    gpio_put(RENCODER_3V3_PIN, HIGH);

    gpio_init(LINFRARED_3V3_PIN);
    gpio_set_dir(LINFRARED_3V3_PIN, GPIO_OUT);
    gpio_put(LINFRARED_3V3_PIN, HIGH);

    gpio_init(RINFRARED_3V3_PIN);
    gpio_set_dir(RINFRARED_3V3_PIN, GPIO_OUT);
    gpio_put(RINFRARED_3V3_PIN, HIGH);

    gpio_init(BINFRARED_3V3_PIN);
    gpio_set_dir(BINFRARED_3V3_PIN, GPIO_OUT);
    gpio_put(BINFRARED_3V3_PIN, HIGH);
}

void motor_control_init()
{
    // Motor A
    gpio_set_function(ENA_PIN, GPIO_FUNC_PWM);
    pwm_set_clkdiv(GP5_SLICE, 100);
    pwm_set_wrap(GP5_SLICE, PWM_FREQ);
    pwm_set_chan_level(GP5_SLICE, PWM_CHAN_B, PWM_FREQ * 0.5);
    pwm_set_enabled(GP5_SLICE, true);

    gpio_init(INA1_PIN);
    gpio_set_dir(INA1_PIN, GPIO_OUT);
    gpio_put(INA1_PIN, LOW);

    gpio_init(INA2_PIN);
    gpio_set_dir(INA2_PIN, GPIO_OUT);
    gpio_put(INA2_PIN, LOW);

    // Motor B
    gpio_set_function(ENB_PIN, GPIO_FUNC_PWM);
    pwm_set_clkdiv(GP0_SLICE, 100);
    pwm_set_wrap(GP0_SLICE, PWM_FREQ);
    pwm_set_chan_level(GP0_SLICE, PWM_CHAN_A, PWM_FREQ * 0.5);
    pwm_set_enabled(GP0_SLICE, true);

    gpio_init(INB1_PIN);
    gpio_set_dir(INB1_PIN, GPIO_OUT);
    gpio_put(INB1_PIN, LOW);

    gpio_init(INB2_PIN);
    gpio_set_dir(INB2_PIN, GPIO_OUT);
    gpio_put(INB2_PIN, LOW);
}

void encoder_init()
{
    gpio_init(LENCODER_PIN);
    gpio_set_dir(LENCODER_PIN, GPIO_IN);

    gpio_init(RENCODER_PIN);
    gpio_set_dir(RENCODER_PIN, GPIO_IN);
}

void infrared_init()
{
    gpio_init(LINFRARED_PIN);
    gpio_set_dir(LINFRARED_PIN, GPIO_IN);

    gpio_init(RINFRARED_PIN);
    gpio_set_dir(RINFRARED_PIN, GPIO_IN);

    gpio_init(BINFRARED_D0_PIN);
    gpio_set_dir(BINFRARED_D0_PIN, GPIO_IN);

    adc_init();
    gpio_init(BINFRARED_PIN);
    gpio_set_dir(BINFRARED_PIN, GPIO_IN);
    adc_select_input(IR_ADC_CHANNEL);
}

void ultrasonic_init()
{
    gpio_init(TRIG_PIN);
    gpio_set_dir(TRIG_PIN, GPIO_OUT);
    gpio_put(TRIG_PIN, LOW);

    gpio_init(ECHO_PIN);
    gpio_set_dir(ECHO_PIN, GPIO_IN);
}