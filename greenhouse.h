/*******************************************************************************
*
*  greenhouse.h -   Manifest Constants and Types for concurrent access to a
*                   circular buffer modelling a message queue.
*
*   Notes:          User defined according to application.
*
*******************************************************************************/

#include "consoleBlock.h"
#include "sensorsBlock.h"

#define N_PLANTS 3
#define BUFSIZE 8
#define NUM_QUEUES 2 + N_PLANTS*6
#define TIMER_Q 0
#define TIMER_TO_CONSOLE_Q 1
#define CONSOLE_Q 0
#define CONTROLLER_Q 1
#define THERMOMETER_Q 2
#define HUM_SENSOR_Q 3
#define PH_SENSOR_Q 4
#define CO2_SENSOR_Q 5

typedef struct {
    int signal;
    float value;
    float value2;
    float value3;
    float value4;
    int sender;
} msg_t;