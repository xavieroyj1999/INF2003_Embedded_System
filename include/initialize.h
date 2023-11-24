void motor_control_init() {
    // Motor A
    gpio_set_function(ENA_PIN, GPIO_FUNC_PWM);
    pwm_set_clkdiv(GP5_SLICE, 100);
    pwm_set_wrap(GP5_SLICE, PWM_FREQ);
    pwm_set_chan_level(GP5_SLICE, PWM_CHAN_B, 0);
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
    pwm_set_chan_level(GP0_SLICE, PWM_CHAN_A, 0);
    pwm_set_enabled(GP0_SLICE, true);

    gpio_init(INB1_PIN);
    gpio_set_dir(INB1_PIN, GPIO_OUT);
    gpio_put(INB1_PIN, LOW);

    gpio_init(INB2_PIN);
    gpio_set_dir(INB2_PIN, GPIO_OUT);
    gpio_put(INB2_PIN, LOW);
}

void encoder_init() {
    gpio_init(LENCODER_PIN);
    gpio_set_dir(LENCODER_PIN, GPIO_IN);

    gpio_init(RENCODER_PIN);
    gpio_set_dir(RENCODER_PIN, GPIO_IN);
}

void magnetometer_init() {
    i2c_init(I2C_PORT, 100 * 1000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    uint8_t const config_continuous[] = {MR_REG_M, CONTINUOUS_MODE};
    i2c_write_blocking(I2C_PORT, MAGNETIC_SENSOR_ADDRESS, config_continuous, 2, false);

    g_initial_degree = magneto_read();
}

void infrared_init() {
    gpio_init(LINFRARED_PIN);
    gpio_set_dir(LINFRARED_PIN, GPIO_IN);

    gpio_init(RINFRARED_PIN);
    gpio_set_dir(RINFRARED_PIN, GPIO_IN);

    gpio_init(BINFRARED_PIN);
    gpio_set_dir(BINFRARED_PIN, GPIO_IN);
}

void ultrasonic_init() {
    gpio_init(TRIG_PIN);
    gpio_set_dir(TRIG_PIN, GPIO_OUT);
    gpio_put(TRIG_PIN, 0);

    gpio_init(ECHO_PIN);
    gpio_set_dir(ECHO_PIN, GPIO_IN);
}