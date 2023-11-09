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