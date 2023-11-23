// General definitions
#define LOW 0
#define HIGH 1
#define DEBOUNCE_TIME 50000 // 50ms

enum directionEvent
{
    STAY = 0x00,
    NORTH = 0x01,
    EAST = 0x02,
    SOUTH = 0x04,
    WEST = 0x08
};

/* For Barcode 39 Decoder
wide bar = 1, narrow bar = 0 */
enum white_bar
{
    add0 = 0b0100,
    add10 = 0b0010,
    add20 = 0b0001,
    add30 = 0b1000,
};

enum black_bar
{
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

// Motor
#define ENB_PIN 0
#define INB1_PIN 1
#define INB2_PIN 2
#define INA1_PIN 3
#define INA2_PIN 4
#define ENA_PIN 5

#define PWM_FREQ 12500
#define GP0_SLICE 0
#define GP5_SLICE 2

// Ultrasonic
#define TRIG_PIN 6
#define ECHO_PIN 7

#define ULTRASONIC_3V3_PIN 8

// Wheel Encoder
#define LENCODER_PIN 14
#define RENCODER_PIN 15

#define LENCODER_3V3_PIN 9
#define RENCODER_3V3_PIN 10

#define ONE_INTERRUPT 1

// Infrared
#define LINFRARED_PIN 16
#define RINFRARED_PIN 17
#define BINFRARED_D0_PIN 20
#define BINFRARED_PIN 26

#define LINFRARED_3V3_PIN 11
#define RINFRARED_3V3_PIN 12
#define BINFRARED_3V3_PIN 13

#define IR_ADC_CHANNEL 0

#define COLOR_THRESHOLD 1500
#define DELIMITER '*'
#define FLIPPED_DELIMITER 'O'

// Buffer
#define mbaTASK_MESSAGE_BUFFER_SIZE (60)
#define WEB_TASK_STACK_SIZE ((configSTACK_DEPTH_TYPE)2048)

// Task Priorities
#define MAIN_TASK (tskIDLE_PRIORITY + 1UL)
#define WEB_TASK_PRIORITY (tskIDLE_PRIORITY + 1UL)