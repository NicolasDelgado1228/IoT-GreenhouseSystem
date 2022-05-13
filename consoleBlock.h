/*******************************************************************************
*
*  consoleBlock.h -   Manifest Constants and Types for concurrent access to a
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
    sSendData,
    sTimeout
} TO_CONSOLE;

typedef enum {
    sSetTimer,
    sTimeExpired,
    sResetTimer
} TO_TIMER;

// STATES

typedef enum {
    IdleConsole,
    WaitingController,
    WaitingTimer
} CONSOLE_STATES;

typedef enum {
    IdleTimer,
    CheckTimeout,
    WastingTime
} TIMER_STATES;