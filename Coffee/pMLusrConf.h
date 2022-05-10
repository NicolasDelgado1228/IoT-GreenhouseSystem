/*******************************************************************************
*
*  pMLusrConf.h -   Manifest Constants and Types for concurrent access to a
*                   circular buffer modelling a message queue
*
*   Notes:          User defined according to application
*
*******************************************************************************/

/***( Manifest constants for fser-defined queuing system  )********************/

#define     BUFSIZE       8     /* number of slots in queues */
#define     NUM_QUEUES    2     /* number of queues */
#define     CONTROLLER_Q  0     /* queue 0: controller */
#define     HARDWARE_Q    1     /* queue 1: hardware */

/***( User-defined message structure )*****************************************/

typedef struct
{
  int   signal;
  int   value;
} msg_t;

/***( User-defined signals )****************************************************/

typedef enum
{
  sCupOfCoffee
} TO_CUSTOMER ;                           /* Signals sent to customer
                                              (environment) */

typedef enum
{
  sCoin,                                  /*  Signals sent */
  sCoffee,                                /*    from customer */
  sTea,                                   /*    to controller */
  sWaterOK,                               /*  Signals sent */
  sCoffeeOK,                              /*    from hardware */
  sWarm                                   /*    to controller */
} TO_CONTROLLER;


typedef enum
{
  sFillWater,
  sFillCoffee,
  sHeatWater
} TO_HARDWARE;                            /* Signals sent from controller
                                            to hardware */

/***( User-defined EFSM states )************************************************/

typedef enum
{
  IdleC,
  PaidTen,
  PouringWater,
  DispensingCoffee,
  BrewingCoffee
} CONTROLLER_STATES;                      /* EFSM states for controller */

typedef enum
{
  IdleH
} HARDWARE_STATES;                        /* EFSM states for hardware */
