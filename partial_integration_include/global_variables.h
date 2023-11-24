uint16_t g_current_left_wheel_count = 0;
uint16_t g_current_right_wheel_count = 0;

uint16_t g_left_encoder_interrupts = 0;
uint16_t g_right_encoder_interrupts = 0;

uint32_t g_distance_travelled = 0;

bool gb_object_detected = false;
bool gb_wall_detected = false;

float g_degree = 0;
float g_initial_degree = 0;
float g_degree_thresholds[4];

int g_barcode_index = 0;
bool g_flipped_barcode = false;
char g_decoded_value = '\0';

static MessageBufferHandle_t g_send_choose_duty_cycle_buffer;
static MessageBufferHandle_t g_receive_choose_duty_cycle_buffer;

static MessageBufferHandle_t g_send_sync_duty_cycle_buffer;
static MessageBufferHandle_t g_receive_sync_duty_cycle_buffer;

static MessageBufferHandle_t g_leftWheelBuffer;
static MessageBufferHandle_t g_rightWheelBuffer;

static SemaphoreHandle_t g_left_wheel_task_complete;
static SemaphoreHandle_t g_right_wheel_task_complete;
static SemaphoreHandle_t g_barcode_task_sempahore;
static SemaphoreHandle_t g_main_task_semaphore;
static SemaphoreHandle_t g_magnetometer_task;
static SemaphoreHandle_t g_straight_path_task_semaphore;
static SemaphoreHandle_t g_object_semaphore;
static SemaphoreHandle_t g_wall_semaphore;
static SemaphoreHandle_t g_barcode_task_complete;