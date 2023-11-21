uint16_t g_current_left_wheel_count = 0;
uint16_t g_current_right_wheel_count = 0;

uint16_t g_left_encoder_interrupts = 0;
uint16_t g_right_encoder_interrupts = 0;

uint32_t g_distance_travelled = 0;

bool g_object_detected = false;
bool g_wall_detected = false;

float g_degree = 0;
float g_initial_degree = 0;
float g_degree_thresholds[4];

int g_barcode_index = 0;
bool g_flipped_barcode = false;
char g_decoded_value = '\0';

static MessageBufferHandle_t send_choose_duty_cycle_buffer;
static MessageBufferHandle_t receive_choose_duty_cycle_buffer;

static MessageBufferHandle_t send_sync_duty_cycle_buffer;
static MessageBufferHandle_t receive_sync_duty_cycle_buffer;

static MessageBufferHandle_t g_leftWheelBuffer;
static MessageBufferHandle_t g_rightWheelBuffer;

static SemaphoreHandle_t left_wheel_task_complete;
static SemaphoreHandle_t right_wheel_task_complete;
static SemaphoreHandle_t barcode_task_sempahore;
static SemaphoreHandle_t main_task_semaphore;
static SemaphoreHandle_t magnetometer_task;
static SemaphoreHandle_t straight_path_task_semaphore;
static SemaphoreHandle_t object_semaphore;
static SemaphoreHandle_t wall_semaphore;