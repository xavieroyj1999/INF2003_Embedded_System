#include <stdio.h> 
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h" 
#include "hardware/i2c.h" 
#include "math.h"
 
#define I2C_PIN_SDA 2 
#define I2C_PIN_SCL 3

#define I2C_PORT i2c1

#define RADIANS_TO_DEGREES 57.29577

#define SCALE_FACTOR_ACCELERO 

// Addresses from Documentation (https://cdn-shop.adafruit.com/datasheets/LSM303DLHC.PDF)
static const uint8_t MAGNETIC_SENSOR_ADDRESS = 0x1E; 
static const uint8_t MR_REG_M = 0x02; 
static const uint8_t OUT_X_H_M = 0x03;
static const uint8_t CONTINUOUS_MODE = 0x00;

static const uint8_t ACCELEROMETER_ADDRESS = 0x19;
static const uint8_t CTRL_REG1_A = 0x20;
static const uint8_t OUT_X_L_A = 0x28;
static const uint8_t ENABLE_ACCEL = 0x27;
 
void init_GY_511() { 
    i2c_init(I2C_PORT, 100 * 1000); 
    gpio_set_function(I2C_PIN_SDA, GPIO_FUNC_I2C); 
    gpio_set_function(I2C_PIN_SCL, GPIO_FUNC_I2C); 
    gpio_pull_up(I2C_PIN_SDA); 
    gpio_pull_up(I2C_PIN_SCL); 
} 
 
void change_to_continuous() { 
    uint8_t config_continuous[] = {MR_REG_M, CONTINUOUS_MODE}; 
    i2c_write_blocking(I2C_PORT, MAGNETIC_SENSOR_ADDRESS, config_continuous, 2, false); 
} 

void init_accelerometer() {
    uint8_t config_accel[] = {CTRL_REG1_A, ENABLE_ACCEL};
    i2c_write_blocking(I2C_PORT, ACCELEROMETER_ADDRESS, config_accel, 2, false);
}
 
bool request_magneto_read() { 
    i2c_write_blocking(I2C_PORT, MAGNETIC_SENSOR_ADDRESS, &OUT_X_H_M, 1, true); 
 
    uint8_t buffer[6]; 
    i2c_read_blocking(I2C_PORT, MAGNETIC_SENSOR_ADDRESS, buffer, 6, false); 
 
    int16_t x = (buffer[0] << 8 | buffer[1]); 
    int16_t z = (buffer[2] << 8 | buffer[3]); 
    int16_t y = (buffer[4] << 8 | buffer[5]); 

    printf("Magnetic Field (X, Y, Z): %d, %d, %d\n", x, y, z);

    float degree = atan2(y, x) * RADIANS_TO_DEGREES;
    if (degree < 0) degree += 360;
    printf("Degree: %f\n", degree);
    return true;
}

bool request_accelero_read() {
    i2c_write_blocking(I2C_PORT, ACCELEROMETER_ADDRESS, &OUT_X_L_A, 1, true);

    uint8_t buffer[6];
    i2c_read_blocking(I2C_PORT, ACCELEROMETER_ADDRESS, buffer, 6, false);

    int16_t accel_x = (buffer[1] << 8 | buffer[0]);
    int16_t accel_y = (buffer[3] << 8 | buffer[2]);
    int16_t accel_z = (buffer[5] << 8 | buffer[4]);

    printf("Accelerometer (X, Y, Z): %d, %d, %d\n", accel_x, accel_y, accel_z);

    return true;
}

void start_timer() {
    struct repeating_timer* magneto_timer = (struct repeating_timer*)malloc(sizeof(struct repeating_timer));
    add_repeating_timer_ms(1000, request_magneto_read, NULL, magneto_timer);

    struct repeating_timer* accelero_timer = (struct repeating_timer*)malloc(sizeof(struct repeating_timer));
    add_repeating_timer_ms(1000, request_accelero_read, NULL, accelero_timer);
}
 
int main() { 
    stdio_init_all(); 
    init_GY_511(); 
    change_to_continuous();
    init_accelerometer();
    start_timer();
    
    while (1);
}