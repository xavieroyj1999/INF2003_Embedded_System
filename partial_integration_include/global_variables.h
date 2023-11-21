uint16_t g_current_left_wheel_count = 0;
uint16_t g_current_right_wheel_count = 0;

uint16_t g_left_encoder_interrupts = 0;
uint16_t g_right_encoder_interrupts = 0;

bool g_object_detected = false;
bool g_wall_detected = false;

float g_degree = 0;
float g_initial_degree = 0;
float g_degree_thresholds[4];

int g_barcode_index = 0;
bool g_flipped_barcode = false;
char g_decoded_value = '\0';