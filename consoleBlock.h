/*******************************************************************************
*
*  sensorsBlock.h -   Manifest Constants and Types for concurrent access to a
*                   circular buffer modelling a message queue. Console block
*                   declarations file.
*
*   Notes:          User defined according to application
*
*******************************************************************************/

// SIGNALS

typedef enum {
    sShowData
} TO_CUSTOMER;

typedef enum {
    sSendData
} TO_CONSOLE;

typedef enum {
    sGetData
} TO_SENSORS;

// STATES

typedef enum {
    WaitingController
} CONSOLE_STATES;