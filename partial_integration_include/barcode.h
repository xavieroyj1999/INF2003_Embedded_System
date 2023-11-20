// Barcode 39 wiki for table, off_bit = black_bar, even_bit = white_bar
char code_39_decoder(int odd_bit, int even_bit) {
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

void determine_char(char decoded_value) {
    static char barcode[3] = {'\0'};
    static int barcode_index = 0;
    barcode[barcode_index] = decoded_value;
    barcode_index++;

    if (barcode_index == 3) {
        if (barcode[0] == DELIMITER && barcode[2] == DELIMITER) {
            printf("Decoded Char: %c\n", barcode[1]);
        } else {
            printf("Failed barcode reading: %c%c%c\n", barcode[0], barcode[1], barcode[2]);
        }
        barcode_index = 0;
    }
}