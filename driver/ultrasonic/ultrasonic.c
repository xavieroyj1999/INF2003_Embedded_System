#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/gpio.h"
#include "hardware/timer.h"

#define TRIG_PIN 6
#define ECHO_PIN 7

void init_gpio();
uint64_t time_elapsed();
double calculate_distance();
void print_distance(double distance);
bool repeating_timer_callback(struct repeating_timer *t);
struct repeating_timer timer;
absolute_time_t start, end;

// Callback function for the repeating timer.
bool repeating_timer_callback(struct repeating_timer *t) {
    uint64_t time = time_elapsed();

    /* Speed of sound 343 meters per second
    Back and Forth = 343m/2 = 171.5m/second
    1 second = 1000000 microsecond
    17150cm/1000000 microsecond = 0.01715cm/us*/

    double distance = time * 0.01715;
    print_distance(distance);

    //Interrupt when reach 5cm or less
    if (distance < 5){
        //Cancel repeating_timer for now
        bool cancelled = cancel_repeating_timer(&timer);
        //Idk what action u wan do here but up to u (Joo Wee)
        printf("Yes its less than 5cm");
        //Continue repeating timer
        add_repeating_timer_ms(1000, repeating_timer_callback, NULL, &timer);
    }

    //Return true to indicate that the timer should continue to fire.
    return true;
}

//Configure pins
void init_gpio(){
    gpio_init(TRIG_PIN);
    gpio_set_dir(TRIG_PIN, GPIO_OUT);
    //Set pin as low
    gpio_put(TRIG_PIN, 0);
    gpio_init(ECHO_PIN);
    gpio_set_dir(ECHO_PIN, GPIO_IN);
}

uint64_t time_elapsed(){
    gpio_put(TRIG_PIN, 1);
    busy_wait_us_32(10);
    gpio_put(TRIG_PIN, 0);

    while(gpio_get(ECHO_PIN) == 0);
    absolute_time_t start = get_absolute_time();
    while(gpio_get(ECHO_PIN) == 1);
    uint64_t time = absolute_time_diff_us(start, get_absolute_time());
    return time;
}

void print_distance(double distance){
    printf("Distance: %f cm\n", distance);
}

int main(){
    // Initialize the system and pins.
    stdio_init_all();
    init_gpio();

    // Create a repeating timer that calls repeating_timer_callback.
    add_repeating_timer_ms(1000, repeating_timer_callback, NULL, &timer);

    //Wait forever
    while(1);
}