/*******************************************************************************
*
*  greenhouse.c -      A program implementing the use case greenhouse
*                         for the Greenhouse System model v1
*
*   Notes:                Error checking omitted...
*
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include "phtrdsMsgLyr.h"

// Function to generate a random number in a defined range
float randomFloat (float min, float max){
    return (((double)rand()) / ((double)RAND_MAX)) * (max - min) + min;
}

// Function prototypes
static void *pConsole(void *arg);
static void *pController(void *arg);
static void *pThermometer(void *arg);
static void *pHumiditySensor(void *arg);
static void *pPhSensor(void *arg);
static void *pCO2Sensor(void *arg);
static void *pTimer(void *arg);

// SDL system creation
int main (void) {

    pthread_t console_tid;
    pthread_t controller_tid;
    pthread_t thermometer_tid;
    pthread_t humidity_sensor_tid;
    pthread_t ph_sensor_tid;
    pthread_t co2_sensor_tid;
    pthread_t timer_tid;

    // Create queues
    initialiseQueues();

    // Create threads
    pthread_create(&console_tid, NULL, pConsole, NULL);
    pthread_create(&controller_tid, NULL, pController, NULL);
    pthread_create(&thermometer_tid, NULL, pThermometer, NULL);
    pthread_create(&humidity_sensor_tid, NULL, pHumiditySensor, NULL);
    pthread_create(&ph_sensor_tid, NULL, pPhSensor, NULL);
    pthread_create(&co2_sensor_tid, NULL, pCO2Sensor, NULL);
    pthread_create(&timer_tid, NULL, pTimer, NULL);

    // Wait for threads to finish
    pthread_join(console_tid, NULL);
    pthread_join(controller_tid, NULL);
    pthread_join(thermometer_tid, NULL);
    pthread_join(humidity_sensor_tid, NULL);
    pthread_join(ph_sensor_tid, NULL);
    pthread_join(co2_sensor_tid, NULL);
    pthread_join(timer_tid, NULL);

    // Destroy queues
    destroyQueues();

    return 0;
}

// Console thread
static void *pConsole (void *arg) {
    CONSOLE_STATES state, next_state;
    msg_t InMsg, OutMsg;
    float Temp, Hum, pH, CO2;
    int scanFrecuency;
    char line[100];

    next_state = IdleConsole;

    // Ask the user how often to take measurements
    printf("¿Con qué frecuencia desea revisar el estado de las plantas? (segundos) ");
    fflush(stdout);
    fflush(stdin);
    fgets(line, sizeof (line), stdin);
    sscanf(line, "%d", &scanFrecuency);

    for ( ; ; ) {

        state = next_state;

        switch (state) {
            case IdleConsole:
                // Send signal to obtain data
                OutMsg.signal = (int) sGetData;
                printf("Enviando mensaje sGetData desde pConsole a pController...\n");
                sendMessage(&(queue[CONTROLLER_Q]), OutMsg);
                next_state = WaitingController;
                break;
            
            case WaitingController:
                // Receives data from the controller
                InMsg = receiveMessage(&(queue[CONSOLE_Q]));
                switch (InMsg.signal) {
                    case sSendData:

                        // Saves the obtained data
                        Temp = InMsg.value;
                        Hum = InMsg.value2;
                        pH = InMsg.value3;
                        CO2 = InMsg.value4;

                        // Gets the current date and time
                        time_t t = time(NULL);
                        struct tm localTime = *localtime(&t);
                        char dateTime[70];
                        char *format = "%Y-%m-%d %H:%M:%S";
                        strftime(dateTime, sizeof dateTime, format, &localTime);

                        // Displays data to the user
                        printf("ESCANEO DE VARIABLES (%s)\n", dateTime);
                        printf("- Temperatura: %.1f °C\n- Humedad: %.1f %%\n- pH: %.1f\n- CO2: %.1f ppm\n", Temp, Hum, pH, CO2);
                        printf("-----------------------------------------\n");

                        // Sends a signal to the timer to count the waiting time
                        OutMsg.signal = (int) sSetTimer;
                        OutMsg.value = scanFrecuency;
                        printf("Enviando mensaje sSetTimer desde pConsole a pTimer...\n");
                        sendMessage(&(queue[TIMER_Q]), OutMsg);
                        next_state = WaitingTimer;
                        break;
                    default:
                        break;
                }
                break;

            case WaitingTimer:
                // Receives signal from the timer and can continue scanning
                InMsg = receiveMessage(&(queue[CONSOLE_Q]));
                switch (InMsg.signal){
                    case sTimeout:
                        OutMsg.signal = (int) sGetData;
                        printf("Enviando mensaje sGetData desde pConsole a pController...\n");
                        sendMessage(&(queue[CONTROLLER_Q]), OutMsg);
                        next_state = WaitingController;
                        break;
                }
                break;

            default:
                break;
        }
    }

    return NULL;
}

// Controller thread
static void *pController (void *arg) {
    CONTROLLER_STATES state, next_state;
    msg_t InMsg, OutMsg;

    next_state = IdleController;
    float sensorsArray[4];

    for ( ; ; ) {

        state = next_state;
        InMsg = receiveMessage(&(queue[CONTROLLER_Q]));

        switch (state) {

            case IdleController:
                switch (InMsg.signal) {
                    case sGetData:
                        // Request temperature
                        OutMsg.signal = (int) sGetTemp;
                        printf("Enviando mensaje sGetTemp desde pController a pThermometer...\n");
                        sendMessage(&(queue[THERMOMETER_Q]), OutMsg);
                        next_state = WaitingTemp;
                        break;
                    default:
                        break;
                }
                break;

            case WaitingTemp:
                switch (InMsg.signal) {
                    case sSendTemp:
                        // Receives temperature and requests humidity
                        sensorsArray[0] = InMsg.value;
                        OutMsg.signal = (int) sGetHumidity;
                        printf("Enviando mensaje sGetHumidity desde pController a pHumiditySensor...\n");
                        sendMessage(&(queue[HUM_SENSOR_Q]), OutMsg);
                        next_state = WaitingHumidity;
                        break;
                    default:
                        break;
                }
                break;

            case WaitingHumidity:
                switch (InMsg.signal){
                    case sSendHumidity:
                        // Receives humidity and requests pH
                        sensorsArray[1] = InMsg.value;
                        OutMsg.signal = (int) sGetPh;
                        printf("Enviando mensaje sGetPh desde pController a pPhSensor...\n");
                        sendMessage(&(queue[PH_SENSOR_Q]), OutMsg);
                        next_state = WaitingPh;
                        break;
                    default:
                        break;
                }
                break;

            case WaitingPh:
                switch (InMsg.signal){
                    case sSendPh:
                        // Receives pH and requests CO2
                        sensorsArray[2] = InMsg.value;
                        OutMsg.signal = (int) sGetCO2;
                        printf("Enviando mensaje sGetCO2 desde pController a pCO2Sensor...\n");
                        sendMessage(&(queue[CO2_SENSOR_Q]), OutMsg);
                        next_state = WaitingCO2;
                        break;
                    default:
                        break;
                }
                break;

            case WaitingCO2:
                switch (InMsg.signal){
                    case sSendCO2:
                        // Receives CO2 and sends the four variables to the console
                        sensorsArray[3] = InMsg.value;
                        OutMsg.signal = (int) sSendData;
                        OutMsg.value = sensorsArray[0];
                        OutMsg.value2 = sensorsArray[1];
                        OutMsg.value3 = sensorsArray[2];
                        OutMsg.value4 = sensorsArray[3];
                        printf("Enviando mensaje sSendData desde pController a pConsole con los valores %.1f, %.1f, %.1f y %.1f...\n", OutMsg.value, OutMsg.value2, OutMsg.value3, OutMsg.value4);
                        sendMessage(&(queue[CONSOLE_Q]), OutMsg);
                        next_state = IdleController;
                        break;
                    default:
                        break;
                }
                break;

            default:
                break;
        }
    }

    return NULL;
}

// Thermometer thread
static void *pThermometer (void *arg) {
    THERMOMETER_STATES state, next_state;
    msg_t InMsg, OutMsg;
    float Temp;

    next_state = IdleTemp;

    for ( ; ; ) {

        state = next_state;
        InMsg = receiveMessage(&(queue[THERMOMETER_Q]));
        Temp = randomFloat(25, 27);

        switch (state) {
            case IdleTemp:
                switch (InMsg.signal) {
                    case sGetTemp:
                        OutMsg.signal = (int) sSendTemp;
                        OutMsg.value = Temp;
                        printf("Enviando mensaje sSendTemp desde pThermometer a pController con el valor %.1f...\n", Temp);
                        sendMessage(&(queue[CONTROLLER_Q]), OutMsg);
                        next_state = IdleTemp;
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }

    return NULL;
}

// Humidity sensor thread
static void *pHumiditySensor (void *arg) {
    HUMIDITY_STATES state, next_state;
    msg_t InMsg, OutMsg;
    float Hum;

    next_state = IdleHum;

    for ( ; ; ) {

        state = next_state;
        InMsg = receiveMessage(&(queue[HUM_SENSOR_Q]));
        Hum = randomFloat(85, 87);

        switch (state) {
            case IdleHum:
                switch (InMsg.signal) {
                    case sGetHumidity:
                        OutMsg.signal = (int) sSendHumidity;
                        OutMsg.value = Hum;
                        printf("Enviando mensaje sSendHumidity desde pHumiditySensor a pController con el valor %.1f...\n", Hum);
                        sendMessage(&(queue[CONTROLLER_Q]), OutMsg);
                        next_state = IdleHum;
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }

    return NULL;
}

// pH sensor thread
static void *pPhSensor (void *arg) {
    PH_STATES state, next_state;
    msg_t InMsg, OutMsg;
    float pH;

    next_state = IdlePh;

    for ( ; ; ) {

        state = next_state;
        InMsg = receiveMessage(&(queue[PH_SENSOR_Q]));
        pH = randomFloat(6.3, 6.7);

        switch (state) {
            case IdlePh:
                switch (InMsg.signal) {
                    case sGetPh:
                        OutMsg.signal = (int) sSendPh;
                        OutMsg.value = pH;
                        printf("Enviando mensaje sSendPh desde pPhSensor a pController con el valor %.1f...\n", pH);
                        sendMessage(&(queue[CONTROLLER_Q]), OutMsg);
                        next_state = IdlePh;
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }

    return NULL;
}

// CO2 sensor thread
static void *pCO2Sensor (void *arg) {
    CO2_STATES state, next_state;
    msg_t InMsg, OutMsg;
    float CO2;

    next_state = IdleCO2;

    for ( ; ; ) {

        state = next_state;
        InMsg = receiveMessage(&(queue[CO2_SENSOR_Q]));
        CO2 = randomFloat(1150, 1250);

        switch (state) {
            case IdleCO2:
                switch (InMsg.signal) {
                    case sGetCO2:
                        OutMsg.signal = (int) sSendCO2;
                        OutMsg.value = CO2;
                        printf("Enviando mensaje sSendCO2 desde pCO2Sensor a pController con el valor %.1f...\n", CO2);
                        sendMessage(&(queue[CONTROLLER_Q]), OutMsg);
                        next_state = IdleCO2;
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }

    return NULL;
}

// Timer thread
static void *pTimer (void *arg) {
    TIMER_STATES state, next_state;
    msg_t InMsg, OutMsg;
    
    next_state = IdleTimer;

    for ( ; ; ) {

        state = next_state;
        InMsg = receiveMessage(&(queue[TIMER_Q]));
        
        switch ( state ) {

            case IdleTimer:
                switch (InMsg.signal) {
                    case sSetTimer:
                        sleep(InMsg.value);
                        OutMsg.signal = (int) sTimeExpired;
                        OutMsg.value = 0;
                        printf("Enviando mensaje sTimeExpired desde pTimer a sí mismo con el valor 0...\n");
                        sendMessage(&(queue[TIMER_Q]), OutMsg);
                        next_state = CheckTimeout;
                        break;
                    default:
                        break;
                }
                break;
            
            case CheckTimeout:
                switch (InMsg.signal) {
                    case sTimeExpired:
                        OutMsg.signal = (int) sTimeout;
                        OutMsg.value = 0;
                        printf("Enviando mensaje sTimeout desde pTimer a pConsole con el valor 0...\n");
                        sendMessage(&(queue[CONSOLE_Q]), OutMsg);
                        next_state = IdleTimer;
                        break;
                    case sResetTimer:
                        next_state = WastingTime;
                        break;
                    default:
                        break;
                    }
                break;
            
            case WastingTime:
                switch(InMsg.signal) {
                    case sTimeout:
                        next_state = IdleTimer;
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }
    
    return NULL;
}