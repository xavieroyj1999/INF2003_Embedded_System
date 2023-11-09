float magneto_read() {
        uint8_t const register_address = OUT_X_H_M;
        i2c_write_blocking(I2C_PORT, MAGNETIC_SENSOR_ADDRESS, &register_address, 1, true); 

        uint8_t buffer[6]; 
        i2c_read_blocking(I2C_PORT, MAGNETIC_SENSOR_ADDRESS, buffer, 6, false); 

        int16_t x = (buffer[0] << 8 | buffer[1]); 
        int16_t y = (buffer[4] << 8 | buffer[5]);

        float g_degree = atan2(y, x) * RADIANS_TO_DEGREES;
        return g_degree;
}

void magneto_read_task() {
    while(1) {
        vTaskDelay(250);
        g_degree = magneto_read();
    }
}

bool right_angle_turn() {
    float degree_difference = g_degree - g_initial_degree;
    if (degree_difference < 0) degree_difference += 360;
    if (degree_difference > 85 && degree_difference < 275) {
        return true;
    }
    return false;
}