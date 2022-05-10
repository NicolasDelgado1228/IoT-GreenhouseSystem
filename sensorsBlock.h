/*******************************************************************************
*
*  sensorsBlock.h -   Manifest Constants and Types for concurrent access to a
*                   circular buffer modelling a message queue. Sensors block
*                   declarations file.
*
*   Notes:          User defined according to application
*
*******************************************************************************/

// SIGNALS

typedef enum {
    sGetData,
    sSendTemp,
    sSendHumidity,
    sSendPh,
    sSendCO2
} TO_CONTROLLER;

typedef enum {
    sGetTemp
} TO_THERMOMETER;

typedef enum {
    sGetHumidity
} TO_HUMIDITY_SENSOR;

typedef enum {
    sGetPh
} TO_PH_SENSOR;

typedef enum {
    sGetCO2
} TO_CO2_SENSOR;


// STATES

typedef enum {
    IdleController,
    WaitingTemp,
    WaitingHumidity,
    WaitingPh,
    WaitingCO2
} CONTROLLER_STATES;

typedef enum {
    IdleTemp,
} THERMOMETER_STATES;

typedef enum {
    IdleHum,
} HUMIDITY_STATES;

typedef enum {
    IdlePh,
} PH_STATES;

typedef enum {
    IdleCO2,
} CO2_STATES;
