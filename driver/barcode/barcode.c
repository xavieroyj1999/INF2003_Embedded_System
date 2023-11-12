#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

#define CONVERT_TO_MILLISECONDS 1000
#define DEBOUNCE_TIME_MS 50

#define LOW 0
#define HIGH 1


// For Decoder
enum white_bar{
    add0 = 0b0100,
    add10 = 0b0010,
    add20 = 0b0001,
    add30 = 0b1000,
};

// For Decoder
enum black_bar{
    no1 = 0b10001,
    no2 = 0b01001,
    no3 = 0b11000,
    no4 = 0b00101,
    no5 = 0b10100,
    no6 = 0b01100,
    no7 = 0b00011,
    no8 = 0b10010,
    no9 = 0b01010,
    no10 = 0b00110,
};

// Barcode 39 wiki for table, off_bit = black_bar, even_bit = white_bar
char code_39_decoder(int odd_bit, int even_bit) {
    printf("odd_bit: %d, even_bit: %d\n", odd_bit, even_bit);
    char pre_defined_code_39_values[40] = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ-. *";

    int index_of_code_39 = 0;
    switch (even_bit) {
        case add0:
            index_of_code_39 += 0;
            break;
        case add10:
            index_of_code_39 += 10;
            break;
        case add20:
            index_of_code_39 += 20;
            break;
        case add30:
            index_of_code_39 += 30;
            break;
    }

    switch (odd_bit) {
        case no1:
            index_of_code_39 += 0;
            break;
        case no2:
            index_of_code_39 += 1;
            break;
        case no3:
            index_of_code_39 += 2;
            break;
        case no4:
            index_of_code_39 += 3;
            break;
        case no5:
            index_of_code_39 += 4;
            break;
        case no6:
            index_of_code_39 += 5;
            break;
        case no7:
            index_of_code_39 += 6;
            break;
        case no8:
            index_of_code_39 += 7;
            break;
        case no9:
            index_of_code_39 += 8;
            break;
        case no10:
            index_of_code_39 += 9;
            break;
    }
    return pre_defined_code_39_values[index_of_code_39];
}

void init_IR_barcode() {
    adc_init();
    gpio_init(26);
    gpio_set_dir(26, GPIO_IN);

    gpio_init(22);
    gpio_set_dir(22, GPIO_OUT);
    gpio_put(22, HIGH); 
}

uint8_t determine_odd_bit(uint32_t odd_time[]) {
    int highest_indices[2] = {0, 1};
    uint32_t highest_times[2] = {odd_time[0], odd_time[1]};

    // Find the two highest times and their indices
    for (int i = 2; i < 5; i++) {
        if (odd_time[i] > highest_times[0]) {
            highest_times[1] = highest_times[0];
            highest_indices[1] = highest_indices[0];
            highest_times[0] = odd_time[i];
            highest_indices[0] = i;
        } else if (odd_time[i] > highest_times[1]) {
            highest_times[1] = odd_time[i];
            highest_indices[1] = i;
        }
    }

    // Combine the two highest bits and return the result
    return (1 << (4 - highest_indices[0])) | (1 << (4- highest_indices[1]));
}

uint8_t determine_even_bit(uint32_t even_time[]) {
    int highest_index = 0;
    uint32_t highest_time = even_time[0];
    for (int i = 1; i < 4; i++) {
        if (even_time[i] > highest_time) {
            highest_time = even_time[i];
            highest_index = i;
        }
    }
    return (1 << (3 - highest_index));
}

void handle_IR_interrupt(uint gpio, uint32_t events) {
    static bool last_edge_was_rising = false;
    static uint32_t last_edge_time = 0;
    uint32_t current_time = time_us_32();
    static uint8_t odd_bit_index = 0;
    static uint8_t even_bit_index = 0;
    static uint32_t odd_time[5] = {0,0,0,0,0};
    static uint32_t even_time[4] = {0,0,0,0};

    // Debounce
    if (current_time - last_edge_time < DEBOUNCE_TIME_MS * CONVERT_TO_MILLISECONDS) {
        return;
    }

    // If rising edge, take time interval between rising and falling edge, then store the bit that will be << based on odd_bit_index
    if (events & GPIO_IRQ_EDGE_RISE && (odd_bit_index == 0)) {
        last_edge_time = time_us_32();
        last_edge_was_rising = true;
    }
    else if (events & GPIO_IRQ_EDGE_RISE && !last_edge_was_rising) {
        uint32_t time_interval = current_time - last_edge_time;
        even_time[even_bit_index] = time_interval;
        even_bit_index++;
        printf("Even Bit: %d\n", even_bit_index);
        printf("Even Array: %d, %d, %d, %d\n", even_time[0], even_time[1], even_time[2], even_time[3]);
        last_edge_was_rising = true;
    }
    else if (events & GPIO_IRQ_EDGE_FALL && last_edge_was_rising) {
        uint32_t time_interval = current_time - last_edge_time;
        odd_time[odd_bit_index] = time_interval;
        odd_bit_index++;
        printf("Odd Bit: %d\n", odd_bit_index);
        printf("Odd Array: %d, %d, %d, %d, %d\n", odd_time[0], odd_time[1], odd_time[2], odd_time[3], odd_time[4]);
        last_edge_was_rising = false;
    }

    // If we have 9 bits, reset everything and decode the value
    if ((odd_bit_index + even_bit_index) >= 9) {
        uint8_t odd_bit = determine_odd_bit(odd_time);
        uint8_t even_bit = determine_even_bit(even_time);
        char decoded_value = code_39_decoder(odd_bit, even_bit);
        odd_bit_index = even_bit_index = 0;
        last_edge_was_rising = false;
        printf("Decoded Value: %c\n", decoded_value);
    }
    last_edge_time = current_time;
}

int main() {
    stdio_init_all();
    init_IR_barcode();
    gpio_set_irq_enabled_with_callback(26, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &handle_IR_interrupt);

    while(1);
    return 0;
}
