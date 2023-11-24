double calculate_distance(uint64_t time_elapsed) {
    /* Speed of sound 343 meters per second
    Back and Forth = 343m/2 = 171.5m/second
    1 second = 1000000 microsecond
    17150cm/1000000 microsecond = 0.01715cm/us */

    double distance = (double) time_elapsed * 0.01715;
    printf("Distance: %f cm\n", distance);
    return distance;
}

void generate_ultrasonic_task(void *pvParameters)
{
    while (true)
    {
        vTaskDelay(100);
        gpio_put(TRIG_PIN, HIGH);
        vTaskDelay(1);
        gpio_put(TRIG_PIN, LOW);
    }
}