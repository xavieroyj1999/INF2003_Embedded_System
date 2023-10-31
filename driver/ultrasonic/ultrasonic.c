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
struct repeating_timer g_timer;

// Callback function for the repeating timer.
bool repeating_timer_callback(struct repeating_timer *t) {
    uint64_t time = time_elapsed();

    /* Speed of sound 343 meters per second
    Back and Forth = 343m/2 = 171.5m/second
    1 second = 1000000 microsecond
    17150cm/1000000 microsecond = 0.01715cm/us */

    double distance = time * 0.01715;
    print_distance(distance);

    //Interrupt when reach 5cm or less
    if (distance < 5){
        //Cancel repeating_timer for now
        bool cancelled = cancel_repeating_timer(&g_timer);
        //Stop car from crashing
        printf("Yes its less than 5cm\n");
        //Continue repeating timer
        add_repeating_timer_ms(1000, repeating_timer_callback, NULL, &g_timer);
    }

    //Return true to indicate that the timer should continue to fire.
    return true;
}

//Configure pins
void init_gpio(){
    gpio_init(TRIG_PIN);
    gpio_set_dir(TRIG_PIN, GPIO_OUT);
    //Set pin as low, dont output yet
    gpio_put(TRIG_PIN, 0);

    gpio_init(ECHO_PIN);
    gpio_set_dir(ECHO_PIN, GPIO_IN);
}

uint64_t time_elapsed(){
    //Sending sound
    gpio_put(TRIG_PIN, 1);
    //For 10 microsecond
    busy_wait_us_32(10);
    //Stop sending
    gpio_put(TRIG_PIN, 0);
    
    //When start listening
    while(gpio_get(ECHO_PIN) == 0);
    //Get time when first listen
    absolute_time_t start = get_absolute_time();
    //Wait receive input
    while(gpio_get(ECHO_PIN) == 1);
    //Get time taken for output and recieve input
    uint64_t time = absolute_time_diff_us(start, get_absolute_time());
    //Return time
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
    add_repeating_timer_ms(1000, repeating_timer_callback, NULL, &g_timer);

    //Wait forever
    while(1);
}