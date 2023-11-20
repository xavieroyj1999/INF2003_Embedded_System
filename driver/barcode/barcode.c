#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "FreeRTOS.h"
#include "task.h"

#include <FreeRTOSConfig.h>

#define CONVERT_TO_MILLISECONDS 1000
#define DEBOUNCE_TIME_MS 50

#define LOW 0
#define HIGH 1

#define IR_PIN 26
#define IR_ADC_CHANNEL 0

#define COLOR_THRESHOLD 1500

#define BARCODE_TASK_PRIORITY (tskIDLE_PRIORITY + 1UL)

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
    gpio_init(IR_PIN);
    gpio_set_dir(IR_PIN, GPIO_IN);
    adc_select_input(IR_ADC_CHANNEL);
}

uint8_t determine_odd_bit(uint16_t odd_count[]) {
    int highest_indices[2] = {0, 1};
    uint16_t highest_count[2] = {odd_count[0], odd_count[1]};

    // Find the two highest times and their indices
    for (int i = 2; i < 5; i++) {
        if (odd_count[i] > highest_count[0]) {
            highest_count[1] = highest_count[0];
            highest_indices[1] = highest_indices[0];
            highest_count[0] = odd_count[i];
            highest_indices[0] = i;
        } else if (odd_count[i] > highest_count[1]) {
            highest_count[1] = odd_count[i];
            highest_indices[1] = i;
        }
    }

    // Combine the two highest bits and return the result
    return (1 << (4 - highest_indices[0])) | (1 << (4- highest_indices[1]));
}

uint8_t determine_even_bit(uint16_t even_count[]) {
    int highest_index = 0;
    uint16_t highest_count = even_count[0];
    for (int i = 1; i < 4; i++) {
        if (even_count[i] > highest_count) {
            highest_count = even_count[i];
            highest_index = i;
        }
    }
    return (1 << (3 - highest_index));
}

char determine_char(char decoded_value) {
    static char barcode[3] = {'\0'};
    static int barcode_index = 0;
}

void read_barcode(void* pvParameters) {
    // If rising edge, take time interval between rising and falling edge, then store the bit that will be << based on odd_bit_index
    bool start_count = false;
    bool last_color_black = true;
    uint8_t odd_bit_index = 0;
    uint8_t even_bit_index = 0;
    uint16_t black_count = 0;
    uint16_t white_count = 0;
    uint16_t odd_count[5] = {0,0,0,0,0};
    uint16_t even_count[4] = {0,0,0,0};
    uint32_t adc_value = 0;
    while(true) {
        adc_value = adc_read();
        if (odd_bit_index == 0 && even_bit_index == 0 && adc_value > COLOR_THRESHOLD) {
            start_count = true;
        }

        if (start_count) {
            if (adc_value >= COLOR_THRESHOLD) {
                black_count++;
            } else if (adc_value < COLOR_THRESHOLD) {
                white_count++;
            }

            if(adc_value >= COLOR_THRESHOLD && !last_color_black) {
                even_count[even_bit_index] = white_count;
                printf("white_count: %d\n", white_count);
                even_bit_index++;
                black_count = 0;
                last_color_black = true;
            } else if (adc_value < COLOR_THRESHOLD && last_color_black) {
                odd_count[odd_bit_index] = black_count;
                printf("black_count: %d\n", black_count);
                odd_bit_index++;
                white_count = 0;
                last_color_black = false;
            }
        }

        // If we have 9 bits, reset everything and decode the value
        if ((odd_bit_index + even_bit_index) >= 9) {
            uint8_t odd_bit = determine_odd_bit(odd_count);
            uint8_t even_bit = determine_even_bit(even_count);
            char decoded_value = code_39_decoder(odd_bit, even_bit);
            odd_bit_index = even_bit_index = 0;
            black_count = white_count = 0;
            start_count = false;
            last_color_black = true;
            determine_char(decoded_value);
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

int main() {
    stdio_init_all();
    init_IR_barcode();

    TaskHandle_t barcode_task;
    xTaskCreate(read_barcode, "temp thread", configMINIMAL_STACK_SIZE, NULL, BARCODE_TASK_PRIORITY, &barcode_task);
    vTaskStartScheduler();
    while(1);
    return 0;
}
