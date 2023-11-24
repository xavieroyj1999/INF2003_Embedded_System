#include <stdio.h> 
#include <stdlib.h>
#include "pico/stdlib.h" 
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "message_buffer.h"
#include "task.h"
#include "math.h"

#include <declarations.h>
#include <global_variables.h>
#include <function_proto.h>
#include <initialize.h>
#include <ultrasonic.h>
#include <barcode.h>
#include <interrupt.h>
#include <magnometer.h>
#include <init_task.h>
#include <motor_control.h>

int main()
{
    stdio_init_all();

    motor_control_init();
    encoder_init();
    magnetometer_init();

    start_interrupts();
    start_tasks();

    return 0;
}