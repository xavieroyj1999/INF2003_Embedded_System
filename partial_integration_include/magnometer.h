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

// Generate 4 thresholds for magnometer readings
void generateDegreeThresholds() {
    while (g_initial_degree > -180) {
        g_initial_degree -= 90;
    }

    for (int i = 0; i < 4; i++) {
        g_initial_degree += 90;
        g_degree_thresholds[i] = g_initial_degree;
    }
}

float findNearestDegreeDifference(float my_degree) {
    printf("My degree: %f\n", my_degree);
    float current_lowest_difference = 360;
    float my_degree_inverse = my_degree - 360;
    bool is_negative_difference = false;
    bool is_negative_difference_selected = false;

    for (int i = 0; i < 4; i++) {
        float my_degree_difference = my_degree - g_degree_thresholds[i];
        if (my_degree_difference < 0) {
            is_negative_difference = true;
            my_degree_difference = -my_degree_difference;
        } else {
            is_negative_difference = false;
        }
        if (my_degree_difference < current_lowest_difference) {
            is_negative_difference_selected = is_negative_difference;
            current_lowest_difference = my_degree_difference;
        }

        float my_degree_inverse_difference = my_degree_inverse - g_degree_thresholds[i];
        if (my_degree_inverse_difference < 0) {
            is_negative_difference = true;
            my_degree_inverse_difference = -my_degree_inverse_difference;
        } else {
            is_negative_difference = false;
        }
        if (my_degree_inverse_difference < current_lowest_difference) {
            is_negative_difference_selected = is_negative_difference;
            current_lowest_difference = my_degree_inverse_difference;
        }
    }

    if (is_negative_difference_selected) {
        return current_lowest_difference;
    } else {
        return -current_lowest_difference;
    }
}