// ssi.h
u16_t ssi_handler(int iIndex, char *pcInsert, int iInsertLen);
void ssi_init();

// cgi.h
const char *cgi_synchronise_wheels_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char *cgi_start_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
void cgi_init(void);

// initialize_pins.h
void high_pin_init();
void motor_control_init();
void encoder_init();
void infrared_init();
void ultrasonic_init();

// motor_control.h
void left_wheel_forward();
void left_wheel_backward();
void left_wheel_stop();

void right_wheel_forward();
void right_wheel_backward();
void right_wheel_stop();

void set_wheel_duty_cycle(double duty_cycle_ratio);
void sync_wheel_task(void *pvParameters);

void choose_duty_cycle_task(void *pvParameters);
void straight_path_task(void *pvParameters);

void left_wheel_task(void *pvParameters);
void right_wheel_task(void *pvParameters);

void send_buffer_to_wheels(enum directionEvent direction);

// ultrasonic.h
double calculate_distance(uint64_t time_elapsed);
void generate_ultrasonic_task(void *pvParameters);

// barcode.h
char code_39_decoder(int odd_bit, int even_bit);
uint8_t determine_odd_bit(uint16_t odd_count[]);
uint8_t determine_even_bit(uint16_t even_count[]);
void determine_char(char decoded_value);
void barcode_task(void *pvParameters);

// interrupts_tasks.h
void interrupt_callback(uint gpio, uint32_t events);
void disable_all_interrupts();
void enable_all_interrupts();
void enable_encoder_interrupts();
void enable_infrared_interrupts();
void enable_ultrasonic_interrupts();
void start_tasks();

// partial_integration.c
void webserver_task(__unused void *params);
void main_task();
int main();