/*******************************************************************************
*
*  pMLusrConf.h -   Manifest Constants and Types for concurrent access to a
*                   circular buffer modelling a message queue
*
*   Notes:          User defined according to application
*
*******************************************************************************/

/***( Manifest constants for user-defined queuing system  )********************/

#define     BUFSIZE       8     /* number of slots in queues */
#define     NUM_QUEUES    2     /* number of queues */
#define     CONTROLLER_Q  0     /* queue 0: controller */
#define     HARDWARE_Q    1     /* queue 1: hardware */

/***( Macros to manage symbolic (literal) enumerated values )******************/

#define     GENERATE_ENUM(ENUM)     ENUM,
#define     GENERATE_STRING(STRING) #STRING,

/***( User-defined message structure )*****************************************/

typedef struct
{
  int   signal;
  int   value;
} msg_t;

/***( User-defined signals )****************************************************/

/* Signals sent to pController1 */

#define FOREACH_TO_CONTROLLER(SIGNAL) \
        SIGNAL(sCoin) \
        SIGNAL(sCoffee) \
        SIGNAL(sTea) \
        SIGNAL(sWaterOk) \
        SIGNAL(sCoffeeOk) \
        SIGNAL(sWarm)

typedef enum
{
  FOREACH_TO_CONTROLLER(GENERATE_ENUM)
} TO_CONTROLLER_ENUM;

static const char *TO_CONTROLLER_STRING[] =
{
  FOREACH_TO_CONTROLLER(GENERATE_STRING)
};

/* Signals sent to pHardware1 */

#define FOREACH_TO_HARDWARE(SIGNAL) \
        SIGNAL(sFillWater)   \
        SIGNAL(sFillCoffee)  \
        SIGNAL(sHeatWater)

typedef enum
{
  FOREACH_TO_HARDWARE(GENERATE_ENUM)
} TO_HARDWARE_ENUM;

static const char *TO_HARDWARE_STRING[] =
{
  FOREACH_TO_HARDWARE(GENERATE_STRING)
};

/***( User-defined EFSM states )************************************************/

/* EFSM states for pController1 */

#define FOREACH_CONTROLLER_STATE(STATE) \
        STATE(IdleC)   \
        STATE(PaidTen)  \
        STATE(PouringWater)  \
        STATE(DispensingCoffee)  \
        STATE(BrewingCoffee)

typedef enum
{
  FOREACH_CONTROLLER_STATE(GENERATE_ENUM)
} CONTROLLER_STATE_ENUM;

static const char *CONTROLLER_STATE_STRING[] =
{
  FOREACH_CONTROLLER_STATE(GENERATE_STRING)
};

/* EFSM states for pHardware1 */

#define FOREACH_HARDWARE_STATE(STATE) \
        STATE(IdleH)

typedef enum
{
  FOREACH_HARDWARE_STATE(GENERATE_ENUM)
} HARDWARE_STATE_ENUM;

static const char *HARDWARE_STATE_STRING[] =
{
  FOREACH_HARDWARE_STATE(GENERATE_STRING)
};
