#include <M5_STHS34PF80.h>

#include "pomodoro.h"

namespace presence {

M5_STHS34PF80 TMOS;

bool presenceState = false;
bool presenceMoved = false;

volatile unsigned long lastPrecenseTime = 0;

void setup() 
{
    while(TMOS.begin(&Wire, STHS34PF80_I2C_ADDRESS, 2, 1) == false) {
        printf("PRI not found\n\r");
        delay(1000);
    }

    TMOS.setTmosODR(STHS34PF80_TMOS_ODR_AT_2Hz);

    TMOS.setPresenceThreshold(200);
    TMOS.setMotionThreshold(200);
    TMOS.setPresenceHysteresis(0x32);
    TMOS.setMotionHysteresis(0x32);

    // TMOS.setGainMode(STHS34PF80_GAIN_WIDE_MODE);
    TMOS.setGainMode(STHS34PF80_GAIN_DEFAULT_MODE);

    TMOS.setTmosSensitivity(0xff);
    TMOS.resetAlgo();
}

void update()
{
    sths34pf80_tmos_drdy_status_t dataReady;

    TMOS.getDataReady(&dataReady);

    if (dataReady.drdy == 1) {
        sths34pf80_tmos_func_status_t status;
        int16_t presenceVal = 0, motionVal = 0;

        TMOS.getPresenceValue(&presenceVal);
        TMOS.getMotionValue(&motionVal);
        TMOS.getStatus(&status);

        printf("Presence: %i, %i. Motion: %i, %i\n\r", status.pres_flag, presenceVal, status.mot_flag, motionVal);

        // if (status.pres_flag == 1 || status.mot_flag == 1) {
        if (status.pres_flag == 1) {
            printf("User found!\n\r");
            presenceState = true;
            lastPrecenseTime = millis();
        }

        // if (status.pres_flag == 0 && status.mot_flag == 0)  {
        if (status.pres_flag == 0)  {
            printf("User NOT found!\n\r");
            presenceMoved = true;
        }
    }
}

void clearState() 
{
    presenceState = false;
}

bool getState()
{
    return presenceState;
}

void clearMovedState()
{
    presenceMoved = false;
}

bool getMovedState() {
    return presenceMoved;
}

unsigned long getPresenceTime()
{
    return lastPrecenseTime;
}


}