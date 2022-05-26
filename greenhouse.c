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

/*  MODE
    0: User -> displays the results table
    1: Debug -> displays message sending
    2: User and debug -> combines user and debug modes
*/

#define MODE 0

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
    pthread_t timer_tid;

    // Create queues
    initialiseQueues();

    // Create threads
    pthread_create(&console_tid, NULL, pConsole, NULL);
    pthread_create(&timer_tid, NULL, pTimer, NULL);

    // Wait for threads to finish
    pthread_join(console_tid, NULL);
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

    int scanFrecuency, i;
    char line[100];

    time_t t;
    struct tm localTime;
    char dateTime[70];
    char *format = "%d-%m-%Y %H:%M:%S";

    pthread_t controllers[N_PLANTS];
    pthread_t thermometers[N_PLANTS];
    pthread_t humiditySensors[N_PLANTS];
    pthread_t phSensors[N_PLANTS];
    pthread_t co2Sensors[N_PLANTS];

    // Create threads
    for (i = 0; i < N_PLANTS; i++) {
        pthread_create(&controllers[i], NULL, pController, (void *) &i);
        pthread_create(&thermometers[i], NULL, pThermometer, (void *) &i);
        pthread_create(&humiditySensors[i], NULL, pHumiditySensor, (void *) &i);
        pthread_create(&phSensors[i], NULL, pPhSensor, (void *) &i);
        pthread_create(&co2Sensors[i], NULL, pCO2Sensor, (void *) &i);
        sleep(1);
    }

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
                for (i = 0; i < N_PLANTS; i++){
                    OutMsg.signal = (int) sGetData;
                    if (MODE == 1 || MODE == 2) printf("Enviando mensaje sGetData desde pConsole a pController (ID = %d)...\n", i);
                    sendMessage(&(queue[2 + 6*i + CONTROLLER_Q]), OutMsg);
                    next_state = WaitingController;
                }
                break;
            
            case WaitingController:

                // Gets the current date and time
                t = time(NULL);
                localTime = *localtime(&t);
                strftime(dateTime, sizeof dateTime, format, &localTime);

                // Displays data to the user
                if (MODE == 0 || MODE == 2){
                    printf("\n\n");
                    printf("ESCANEO DE VARIABLES (%s)\n", dateTime);
                    printf("------------------------------------------------------------------\n");
                    printf("Planta \tTemperatura\tHumedad\t\tNivel pH\tNivel CO2\n");
                    printf("------------------------------------------------------------------\n");
                }

                for (i = 0; i < N_PLANTS; i++){
                    // Receives data from the controller
                    InMsg = receiveMessage(&(queue[2 + 6*i + CONSOLE_Q]));
                    switch (InMsg.signal) {
                        case sSendData:

                            // Saves the obtained data
                            Temp = InMsg.value;
                            Hum = InMsg.value2;
                            pH = InMsg.value3;
                            CO2 = InMsg.value4;

                            // Displays data to the user
                            if (MODE == 0 || MODE == 2) printf("%d \t%.1f °C\t\t%.1f %%\t\t%.1f\t\t%.1f ppm\n", InMsg.sender, Temp, Hum, pH, CO2);

                            if (i + 1 == N_PLANTS) {
                                // Sends a signal to the timer to count the waiting time
                                OutMsg.signal = (int) sSetTimer;
                                OutMsg.value = scanFrecuency;
                                if (MODE == 1 || MODE == 2) printf("Enviando mensaje sSetTimer desde pConsole a pTimer...\n");
                                sendMessage(&(queue[TIMER_Q]), OutMsg);
                                next_state = WaitingTimer;
                            }
                            break;
                        default:
                            break;
                    }
                }
                break;

            case WaitingTimer:
                // Receives signal from the timer and can continue scanning
                InMsg = receiveMessage(&(queue[TIMER_TO_CONSOLE_Q]));
                switch (InMsg.signal){
                    case sTimeout:

                        for (i = 0; i < N_PLANTS; i++) {
                            OutMsg.signal = (int) sGetData;
                            if (MODE == 1 || MODE == 2) printf("Enviando mensaje sGetData desde pConsole a pController (ID = %d)...\n", i);
                            sendMessage(&(queue[2 + 6*i + CONTROLLER_Q]), OutMsg);
                        }

                        next_state = WaitingController;
                        break;
                }
                break;

            default:
                break;
        }
    }

    // Wait for threads to finish
    for (i = 0; i < N_PLANTS; i++){
        pthread_join(controllers[i], NULL);
        pthread_join(thermometers[i], NULL);
        pthread_join(humiditySensors[i], NULL);
        pthread_join(phSensors[i], NULL);
        pthread_join(co2Sensors[i], NULL);
    }

    return NULL;
}

// Controller thread
static void *pController (void *arg) {
    int *data = (int *) arg;
    int ID = *data;

    CONTROLLER_STATES state, next_state;
    msg_t InMsg, OutMsg;

    next_state = IdleController;
    float sensorsArray[4];

    for ( ; ; ) {

        state = next_state;
        InMsg = receiveMessage(&(queue[2 + 6*ID + CONTROLLER_Q]));

        switch (state) {

            case IdleController:
                switch (InMsg.signal) {
                    case sGetData:
                        // Request temperature
                        OutMsg.signal = (int) sGetTemp;
                        if (MODE == 1 || MODE == 2) printf("Enviando mensaje sGetTemp desde pController (ID = %d) a pThermometer (ID = %d)...\n", ID, ID);
                        sendMessage(&(queue[2 + 6*ID + THERMOMETER_Q]), OutMsg);
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
                        if (MODE == 1 || MODE == 2) printf("Enviando mensaje sGetHumidity desde pController (ID = %d) a pHumiditySensor (ID = %d)...\n", ID, ID);
                        sendMessage(&(queue[2 + 6*ID + HUM_SENSOR_Q]), OutMsg);
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
                        if (MODE == 1 || MODE == 2) printf("Enviando mensaje sGetPh desde pController (ID = %d) a pPhSensor (ID = %d)...\n", ID, ID);
                        sendMessage(&(queue[2 + 6*ID + PH_SENSOR_Q]), OutMsg);
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
                        if (MODE == 1 || MODE == 2) printf("Enviando mensaje sGetCO2 desde pController (ID = %d) a pCO2Sensor (ID = %d)...\n", ID, ID);
                        sendMessage(&(queue[2 + 6*ID + CO2_SENSOR_Q]), OutMsg);
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
                        OutMsg.sender = (int) ID;
                        OutMsg.value = sensorsArray[0];
                        OutMsg.value2 = sensorsArray[1];
                        OutMsg.value3 = sensorsArray[2];
                        OutMsg.value4 = sensorsArray[3];
                        if (MODE == 1 || MODE == 2) printf("Enviando mensaje sSendData desde pController (ID = %d) a pConsole con los valores %.1f, %.1f, %.1f y %.1f...\n", ID, OutMsg.value, OutMsg.value2, OutMsg.value3, OutMsg.value4);
                        sendMessage(&(queue[2 + 6*ID + CONSOLE_Q]), OutMsg);
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
    int *data = (int *) arg;
    int ID = *data;

    THERMOMETER_STATES state, next_state;
    msg_t InMsg, OutMsg;
    float Temp;

    next_state = IdleTemp;

    for ( ; ; ) {

        state = next_state;
        InMsg = receiveMessage(&(queue[2 + 6*ID + THERMOMETER_Q]));
        Temp = randomFloat(25, 27);

        switch (state) {
            case IdleTemp:
                switch (InMsg.signal) {
                    case sGetTemp:
                        OutMsg.signal = (int) sSendTemp;
                        OutMsg.value = Temp;
                        if (MODE == 1 || MODE == 2) printf("Enviando mensaje sSendTemp desde pThermometer (ID = %d) a pController (ID = %d) con el valor %.1f...\n", ID, ID, Temp);
                        sendMessage(&(queue[2 + 6*ID + CONTROLLER_Q]), OutMsg);
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
    int *data = (int *) arg;
    int ID = *data;

    HUMIDITY_STATES state, next_state;
    msg_t InMsg, OutMsg;
    float Hum;

    next_state = IdleHum;

    for ( ; ; ) {

        state = next_state;
        InMsg = receiveMessage(&(queue[2 + 6*ID + HUM_SENSOR_Q]));
        Hum = randomFloat(85, 87);

        switch (state) {
            case IdleHum:
                switch (InMsg.signal) {
                    case sGetHumidity:
                        OutMsg.signal = (int) sSendHumidity;
                        OutMsg.value = Hum;
                        if (MODE == 1 || MODE == 2) printf("Enviando mensaje sSendHumidity desde pHumiditySensor (ID = %d) a pController (ID = %d) con el valor %.1f...\n", ID, ID, Hum);
                        sendMessage(&(queue[2 + 6*ID + CONTROLLER_Q]), OutMsg);
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
    int *data = (int *) arg;
    int ID = *data;

    PH_STATES state, next_state;
    msg_t InMsg, OutMsg;
    float pH;

    next_state = IdlePh;

    for ( ; ; ) {

        state = next_state;
        InMsg = receiveMessage(&(queue[2 + 6*ID + PH_SENSOR_Q]));
        pH = randomFloat(6.3, 6.7);

        switch (state) {
            case IdlePh:
                switch (InMsg.signal) {
                    case sGetPh:
                        OutMsg.signal = (int) sSendPh;
                        OutMsg.value = pH;
                        if (MODE == 1 || MODE == 2) printf("Enviando mensaje sSendPh desde pPhSensor (ID = %d) a pController (ID = %d) con el valor %.1f...\n", ID, ID, pH);
                        sendMessage(&(queue[2 + 6*ID + CONTROLLER_Q]), OutMsg);
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
    int *data = (int *) arg;
    int ID = *data;

    CO2_STATES state, next_state;
    msg_t InMsg, OutMsg;
    float CO2;

    next_state = IdleCO2;

    for ( ; ; ) {

        state = next_state;
        InMsg = receiveMessage(&(queue[2 + 6*ID + CO2_SENSOR_Q]));
        CO2 = randomFloat(1150, 1250);

        switch (state) {
            case IdleCO2:
                switch (InMsg.signal) {
                    case sGetCO2:
                        OutMsg.signal = (int) sSendCO2;
                        OutMsg.value = CO2;
                        if (MODE == 1 || MODE == 2) printf("Enviando mensaje sSendCO2 desde pCO2Sensor (ID = %d) a pController (ID = %d) con el valor %.1f...\n", ID, ID, CO2);
                        sendMessage(&(queue[2 + 6*ID + CONTROLLER_Q]), OutMsg);
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
                        if (MODE == 1 || MODE == 2) printf("Enviando mensaje sTimeExpired desde pTimer a sí mismo con el valor 0...\n");
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
                        if (MODE == 1 || MODE == 2) printf("Enviando mensaje sTimeout desde pTimer a pConsole con el valor 0...\n");
                        sendMessage(&(queue[TIMER_TO_CONSOLE_Q]), OutMsg);
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