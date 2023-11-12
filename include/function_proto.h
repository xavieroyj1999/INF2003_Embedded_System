float magneto_read();
bool back_turn();
bool right_angle_turn();

void left_wheel_forward();
void right_wheel_forward();
void left_wheel_backward();
void right_wheel_backward();
void left_wheel_stop();
void right_wheel_stop();
void start_tasks();
void main_task(void* pvParameters);
void left_wheel_task(void* pvParameters);
void right_wheel_task(void* pvParameters);
void control_wheels_task(void* pvParameters);
void user_selection_task(void* pvParameters);