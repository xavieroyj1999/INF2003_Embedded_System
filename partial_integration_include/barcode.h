/* Barcode 39 wiki for table, odd_bit = black_bar, even_bit = white_bar
the algorithm used to decode the barcode is using the 2 highest duration of the black bar and 1 highest duration of the white bar
as this is universal for all barcode 39, allowing it to scan barcode of any size*/
char code_39_decoder(int odd_bit, int even_bit)
{
    char pre_defined_code_39_values[40] = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ-. *";

    // Check sum for white bars, check definitions for bits value
    int index_of_code_39 = 0;
    switch (even_bit)
    {
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

    // Check sum for black bars
    switch (odd_bit)
    {
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

// Determine the 2 highest bit in odd_count and combine them
uint8_t determine_odd_bit(uint16_t odd_count[])
{
    // If barcode is flipped, reverse the array
    if (g_flipped_barcode)
    {
        uint8_t length = 5;
        uint8_t temp;
        for (int index = 0; index < length / 2; index++)
        {
            temp = odd_count[index];
            odd_count[index] = odd_count[length - index - 1];
            odd_count[length - index - 1] = temp;
        }
    }

    int highest_indices[2] = {0, 1};
    uint16_t highest_count[2] = {odd_count[0], odd_count[1]};

    // Determine the two highest times and their indices
    for (int index = 2; index < 5; index++)
    {
        if (odd_count[index] > highest_count[0])
        {
            highest_count[1] = highest_count[0];
            highest_indices[1] = highest_indices[0];
            highest_count[0] = odd_count[index];
            highest_indices[0] = index;
        }
        else if (odd_count[index] > highest_count[1])
        {
            highest_count[1] = odd_count[index];
            highest_indices[1] = index;
        }
    }

    // Combine the two highest bits and return the result
    return (1 << (4 - highest_indices[0])) | (1 << (4 - highest_indices[1]));
}

// Determine the highest bit in even_count and return it
uint8_t determine_even_bit(uint16_t even_count[])
{
    // If barcode is flipped, reverse the array
    if (g_flipped_barcode)
    {
        uint8_t length = 4;
        uint8_t temp;
        for (int index = 0; index < length / 2; index++)
        {
            temp = even_count[index];
            even_count[index] = even_count[length - index - 1];
            even_count[length - index - 1] = temp;
        }
    }

    int highest_index = 0;
    uint16_t highest_count = even_count[0];

    // Determine the highest time and its index
    for (int index = 1; index < 4; index++)
    {
        if (even_count[index] > highest_count)
        {
            highest_count = even_count[index];
            highest_index = index;
        }
    }
    return (1 << (3 - highest_index));
}

// Determine the character from the decoded value
void determine_char(char decoded_value)
{
    static char barcode[3] = {'\0'};
    barcode[g_barcode_index] = decoded_value;

    // If first char is '*', it is not flipped
    if (g_barcode_index == 0 && decoded_value == DELIMITER)
    {
        g_flipped_barcode = false;
    }
    // If first char is 'O', it is flipped
    else if (g_barcode_index == 0 && decoded_value == FLIPPED_DELIMITER)
    {
        barcode[0] = DELIMITER;
        g_flipped_barcode = true;
    }

    // Upon reaching 3 char, print data and reset
    if (g_barcode_index == 2)
    {
        printf("Barcode reading: %c%c%c\n", barcode[0], barcode[1], barcode[2]);
        printf("Decoded Value: %c\n", barcode[1]);
        g_flipped_barcode = false;
        g_barcode_index = 0;
        g_decoded_value = barcode[1];
        xSemaphoreGive(g_barcode_task_complete);
        return;
    }
    g_barcode_index++;
}

void barcode_task(void *pvParameters)
{
    bool start_count = false;
    bool last_color_black = true;
    uint8_t odd_bit_index = 0;
    uint8_t even_bit_index = 0;
    uint16_t black_count = 0;
    uint16_t white_count = 0;
    uint16_t odd_count[5] = {0, 0, 0, 0, 0};
    uint16_t even_count[4] = {0, 0, 0, 0};
    uint32_t adc_value = 0;
    uint8_t count = 0;
    while (true)
    {
        xSemaphoreTake(g_barcode_task_sempahore, portMAX_DELAY);
        while (count < 3) // Do not exit until 3 barcode char is read
        {
            adc_value = adc_read();
            // First bit is always black, thus wait for black before starting to read
            if (odd_bit_index == 0 && even_bit_index == 0 && adc_value > COLOR_THRESHOLD)
            {
                start_count = true;
            }

            if (start_count)
            {
                if (adc_value >= COLOR_THRESHOLD)
                {
                    black_count++;
                }
                else if (adc_value < COLOR_THRESHOLD)
                {
                    white_count++;
                }

                // If the current color is black and the last color is white, add the white count to the even_count array
                if (adc_value >= COLOR_THRESHOLD && !last_color_black && start_count)
                {
                    even_count[even_bit_index] = white_count;
                    even_bit_index++;
                    black_count = 0;
                    last_color_black = true;
                }
                // Else if the current color is white and the last color is black, add the black count to the odd_count array
                else if (adc_value < COLOR_THRESHOLD && last_color_black && start_count)
                {
                    odd_count[odd_bit_index] = black_count;
                    odd_bit_index++;
                    white_count = 0;
                    last_color_black = false;
                }
            }

            // If we have 9 bits, reset everything and decode the value
            if ((odd_bit_index + even_bit_index) >= 9)
            {
                uint8_t odd_bit = determine_odd_bit(odd_count);
                uint8_t even_bit = determine_even_bit(even_count);
                char decoded_value = code_39_decoder(odd_bit, even_bit);
                odd_bit_index = even_bit_index = 0;
                black_count = white_count = 0;
                start_count = false;
                last_color_black = true;
                determine_char(decoded_value);
                count++;
            }
            vTaskDelay(pdMS_TO_TICKS(1));
        }
        count = 0;
    }
}