void interrupt_callback(uint gpio, uint32_t events) {
    uint32_t current_time = time_us_32();
    if (events & GPIO_IRQ_EDGE_RISE && gpio == RENCODER_PIN) {
        static uint32_t rencoder_last_time = 0;
        if (current_time - rencoder_last_time > DEBOUNCE_TIME) {
            g_distance_travelled += ONE_INTERRUPT;
            rencoder_last_time = current_time;
            g_right_encoder_interrupts++;
        }
    }
    if (events & GPIO_IRQ_EDGE_RISE && gpio == LENCODER_PIN) {
        static uint32_t lencoder_last_time = 0;
        if (current_time - lencoder_last_time > DEBOUNCE_TIME) {
            g_distance_travelled += ONE_INTERRUPT;
            lencoder_last_time = current_time;
            g_left_encoder_interrupts++;
        }
    }
    if (events & GPIO_IRQ_EDGE_RISE && gpio == LINFRARED_PIN) {
        static uint32_t linfrared_last_time = 0;
        if (current_time - linfrared_last_time > DEBOUNCE_TIME) {
            linfrared_last_time = current_time;
        }
        control_wheels(EAST);
    }
    if (events & GPIO_IRQ_EDGE_RISE && gpio == RINFRARED_PIN) {
        static uint32_t rinfrared_last_time = 0;
        if (current_time - rinfrared_last_time > DEBOUNCE_TIME) {
            rinfrared_last_time = current_time;
        }
        control_wheels(WEST);
    }
    if (gpio == BINFRARED_PIN) {
        static bool last_edge_was_rising = false;
        static uint32_t last_edge_time = 0;
        static uint8_t odd_bit_index = 0;
        static uint8_t even_bit_index = 0;
        static uint32_t odd_time[5] = {0,0,0,0,0};
        static uint32_t even_time[4] = {0,0,0,0};

        if (current_time - last_edge_time < DEBOUNCE_TIME) {
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
            last_edge_was_rising = true;
        }
        else if (events & GPIO_IRQ_EDGE_FALL && last_edge_was_rising) {
            uint32_t time_interval = current_time - last_edge_time;
            odd_time[odd_bit_index] = time_interval;
            odd_bit_index++;
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
}