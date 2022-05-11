#include "greenhouse.h"
#include "consoleBlock.h"
#include "sensorsBlock.h"

static void *pConsole(void *arg);
static void *pController(void *arg);
static void *pThermometer(void *arg);
static void *pHumiditySensor(void *arg);
static void *pPhSensor(void *arg);
static void *pCO2Sensor(void *arg);

int main (void) {

    pthread_t thermometer_tid;

    initialiseQueues();

    pthread_create(&console_tid, NULL, pConsole, NULL);
    pthread_create(&controller_tid, NULL, pController, NULL);
    pthread_create(&thermometer_tid, NULL, pThermometer, NULL);
    pthread_create(&humidity_sensor_tid, NULL, pHumiditySensor, NULL);
    pthread_create(&ph_sensor_tid, NULL, pPhSensor, NULL);
    pthread_create(&co2_sensor_tid, NULL, pCO2Sensor, NULL);

    pthread_join(console_tid, NULL);
    pthread_join(controller_tid, NULL);
    pthread_join(thermometer_tid, NULL);
    pthread_join(humidity_sensor_tid, NULL);
    pthread_join(ph_sensor_tid, NULL);
    pthread_join(co2_sensor_tid, NULL);

    destroyQueues();

    return 0;
}

// Controller thread
static void *pController (void *arg) {
    CONTROLLER_STATES state, state_next;
    state_next = IdleController;

    msg_t_array InMsgConsole, OutMsgConsole;

    msg_t InMsgTemp, OutMsgTemp,
        InMsgHum, OutMsgHum,
        InMsgPh, OutMsgPh,
        InMsgCO2, OutMsgCO2;

    float sensorsArray[4];

    for ( ; ; ) {

        state = state_next;

        InMsgConsole = receiveMessage(&(queue[CONSOLE_Q]));
        InMsgTemp = receiveMessage(&(queue[THERMOMETER_Q]));
        InMsgHum = receiveMessage(&(queue[HUM_SENSOR_Q]));
        InMsgPh = receiveMessage(&(queue[PH_SENSOR_Q]));
        InMsgCO2 = receiveMessage(&(queue[CO2_SENSOR_Q]));

        switch (state) {
            case IdleController:
                switch (InMsgConsole.signal) {
                    case sGetData:
                        OutMsgTemp.signal = sGetTemp;
                        state_next = WaitingTemp;
                        break;
                    default:
                        break;
                }
                break;
            case WaitingTemp:
                switch (InMsgTemp.signal) {
                    case sSendTemp:
                        sensorsArray[0] = InMsgTemp.value;
                        OutMsgHum.signal = sGetHumidity;
                        state_next = WaitingHumidity;
                        break;
                    default:
                        break;
                }
                break;
            case WaitingHumidity:
                switch (InMsgHum.signal){
                    case sSendHumidity:
                        sensorsArray[1] = InMsgHum.value;
                        OutMsgPh.signal = sGetPh;
                        state_next = WaitingPh;
                        break;
                    default:
                        break;
                }
                break;
            case WaitingPh:
                switch (InMsgPh.signal){
                    case sSendPh:
                        sensorsArray[2] = InMsgPh.value;
                        OutMsgCO2.signal = sGetCO2;
                        state_next = WaitingCO2;
                        break;
                    default:
                        break;
                }
                break;
            case WaitingCO2:
                switch (InMsgCO2.signal){
                    case sSendCO2:
                        sensorsArray[3] = InMsgCO2.value;
                        OutMsgConsole.signal = sSendData;
                        OutMsgConsole.value = sensorsArray;
                        state_next = IdleController;
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }

        sendMessage(&(queue[CONSOLE_Q], OutMsgConsole));
        sendMessage(&(queue[THERMOMETER_Q], OutMsgTemp));
        sendMessage(&(queue[HUM_SENSOR_Q], OutMsgHum));
        sendMessage(&(queue[PH_SENSOR_Q], OutMsgPh));
        sendMessage(&(queue[CO2_SENSOR_Q], OutMsgCO2));
    }
}

// Thermometer thread
static void *pThermometer (void *arg) {
    THERMOMETER_STATES state, state_next;
    state_next = IdleTemp;
    msg_t InMsg, OutMsg;

    float Temp = 20.3;

    for ( ; ; ) {

        state = state_next;
        InMsg = receiveMessage(&(queue[THERMOMETER_Q]));

        switch (InMsg.signal) {
            case sGetTemp:
                OutMsg.signal = (int) sSendTemp;
                break;
            default:
                break;
        }

        OutMsg.value = Temp;
        sendMessage(&(queue[CONTROLLER_Q], OutMsg));
    }

    return (NULL);
}
