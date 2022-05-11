#define BUFSIZE 8
#define NUM_QUEUES 6
#define CONSOLE_Q 0
#define CONTROLLER_Q 1
#define THERMOMETER_Q 2
#define HUM_SENSOR_Q 3
#define PH_SENSOR_Q 4
#define CO2_SENSOR_Q 5

typedef struct {
    int signal;
    float value;
} msg_t;

typedef struct {
    int signal;
    float value[4];
} msg_t_array;
