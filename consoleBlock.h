/*******************************************************************************
*
*  sensorsBlock.h -   Manifest Constants and Types for concurrent access to a
*                   circular buffer modelling a message queue. Console block
*                   declarations file.
*
*   Notes:          User defined according to application
*
*******************************************************************************/

/***( Manifest constants for fser-defined queuing system  )********************/

#define     BUFSIZE       8     /* number of slots in queues */
#define     NUM_QUEUES    2     /* number of queues */
//#define     CONTROLLER_Q  0     /* queue 0: controller */
//#define     HARDWARE_Q    1     /* queue 1: hardware */

/***( User-defined message structure )*****************************************/

typedef struct
{
  int   signal;
  int   value;
} msg_t;

/***( User-defined signals )****************************************************/

typedef enum
{
    sShowData
} TO_CUSTOMER;

typedef enum
{
    sSendData
} TO_CONSOLE;

typedef enum
{
    sGetData
} TO_SENSORS;

/***( User-defined EFSM states )************************************************/

typedef enum
{
    WaitingController
} CONSOLE_STATES;